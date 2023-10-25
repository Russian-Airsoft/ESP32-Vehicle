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
    Serial.println(F("           (deg)      (deg)       Age  23.10.2023 14:15    Age  (m)    --- from GPS ----  ---- to Point   ----  RX    RX        Fail"));
    Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
}

void loop() {
  if (!authenticated) {
    // Выполнение процедуры аутентификации
    Serial.println("Выполнение процедуры аутентификации");
    authenticate();
  } else {
    // Отправка координат
    Serial.println("Отправка координат");
    sendCoordinates();
    delay(5000); // Пауза между отправками
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
  
  delay(200); // Пауза между отправками
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
      } else {
        Serial.println("Поссылка где-то гуляет, но до сервера так и не дошла((");
      }
    }
  }
}