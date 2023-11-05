#include <WiFi.h>
#include <WiFiUdp.h>

/*Settings Wifi*/
const char *ssid = "TP-Link_7743";
const char *password = "551551551";

/*Settings Server*/
const IPAddress host(192,168,0,193); //IP Vhicel Server
const uint16_t port = 40054; //Port Vhicel Server

/*Settigns Vhicle*/
const char *vehicle_id = "1";
const char *vehicle_password = "0001"; // =/null / unical

WiFiUDP udp;
bool authenticated = false;
bool serverIsNotResponding = false;
void setup() {
      Serial.begin(115200);
      Serial.println("Connect to WiFi...");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);

      Serial.println();
      Serial.println();

      Serial.print("Waiting for WiFi...");

      while(WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          delay(500);
      }


    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // put your setup code here, to run once:
    Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum Odometer"));
    Serial.println(F("           (deg)      (deg)       Age  05.11.2023 17:46    Age  (m)    --- from GPS ----  ---- to Point   ----  RX    RX        Fail"));
    Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
}

void loop() {
  if(!serverIsNotResponding)
  { 
    delay(5000); // Пауза между отправками
    checkingServer();
  }
  else{
    if (!authenticated) {
        // Выполнение процедуры аутентификации
        delay(1000); // Пауза между отправками
        Serial.println("Выполнение процедуры аутентификации");
        authenticate();
    } else {
        // Отправка координат
        Serial.println("Отправка координат");
        sendCoordinates();
        delay(5000); // Пауза между отправками
    }
  }
}

void checkingServer()
{
  // Отправка запроса аутентификации на сервер
  udp.beginPacket(host, port);
  udp.print("PING:");
  udp.endPacket();
  
  delay(1000); // Пауза между отправками
  // Ожидание ответа от сервера
  char buffer[255];
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(buffer, sizeof(buffer));
    if (len > 0) {
      buffer[len] = 0;
      if (strcmp(buffer, "PONG") == 0) {
        serverIsNotResponding = true;
        Serial.println("Сервер работает!");
        checkingAuth();
      } else {
        Serial.println("WARNING! Сервер не отваечат!!!");
      }
    }
  }
  else{
    Serial.println("WARNING! Сервер не включен!!!");
  }
}

void checkingAuth()
{
  // Отправка запроса аутентификации на сервер
  udp.beginPacket(host, port);
  udp.print("CHECK_AUTH:");
  udp.endPacket();
  
  delay(1000); // Пауза между отправками
  // Ожидание ответа от сервера
  char buffer[255];
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(buffer, sizeof(buffer));
    if (len > 0) {
      buffer[len] = 0;
      if (strcmp(buffer, "AUTHENTICATED") == 0) {
        serverIsNotResponding = true;
        Serial.println("Проверка авторизации - Авторизован!");
        authenticated = true;
      }
      else if (strcmp(buffer, "NOT_AUTH") == 0) {
        Serial.println("Проверка авторизации - Не авторизован!");
        authenticated = false;
        authenticate();
      }
    }
  }
  else{
    Serial.println("WARNING! Сервер не включен!!!");
  }
}

void authenticate() {
  // Отправка запроса аутентификации на сервер
  udp.beginPacket(host, port);
  udp.print("AUTH:");
  udp.print(vehicle_id);
  udp.print("/*/");
  udp.print(vehicle_password);
  udp.endPacket();
  
  delay(1000); // Пауза между отправками
  // Ожидание ответа от сервера
  char buffer[255];
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(buffer, sizeof(buffer));
    if (len > 0) {
      buffer[len] = 0;
      if (strcmp(buffer, "AUTHENTICATED") == 0) {
        authenticated = true;
        Serial.println("Аутентификация прошла успешно");
      } else {
        Serial.println("Аутентификация не удалась");
      }
    }
  }
  else serverIsNotResponding = true;
}

void sendCoordinates() {
  float longitude = 12.345614; // Ваши координаты
  float latitude = 78.901257;  // Ваши координаты
  float alt = 32.40;
  float course = 254.3;
  float speed = 53.4;
  float odometer = 55.7;
  enum DriveTypes {stop, drive};
  DriveTypes driveStatus = drive;
  // Отправка координат на сервер
  udp.beginPacket(host, port);
  udp.print("COORDS:");
  udp.print(longitude, 6);
  udp.print("/*/");
  udp.print(latitude, 6);
  udp.print("/*/");
  udp.print(alt, 3);
  udp.print("/*/");
  udp.print(course, 1);
  udp.print("/*/");
  udp.print(speed, 1);
  udp.print("/*/");
  udp.print(odometer, 1);
  udp.print("/*/");
  udp.print(driveStatus);
  udp.endPacket();


  delay(200); // Пауза между отправками
  // Ожидание ответа от сервера
  char buffer[255];
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(buffer, sizeof(buffer));
    if (len > 0) {
      buffer[len] = 0;
      if (strcmp(buffer, "GEOLOCATIONSET") == 0) {
        Serial.println("Данные успешно отправленны");
      }
      else if (strcmp(buffer, "NOT_AUTH") == 0) {
        Serial.println("WARNING! Данные не отравлены!!! Вы не авторизированы!");
        authenticated = false;
      } else {
        Serial.println("WARNING! Поссылка где-то гуляет, но до сервера так и не дошла((");
      }
    }
  }
  else{
    Serial.println("WARNING! Server не отвечает!");
    serverIsNotResponding = true;
  }
}