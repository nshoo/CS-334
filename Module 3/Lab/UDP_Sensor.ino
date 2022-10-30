#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

#define PHOTO_PIN 34
#define PIEZO_PIN 35

void sendMessage();

WiFiUDP udp;

const char* ssid = "yale wireless";

const char * udpAddress = "172.29.16.222";
const int udpPort = 8888;

void setup()
{
  pinMode(PHOTO_PIN, INPUT);
  pinMode(PIEZO_PIN, INPUT);
  
    Serial.begin(115200);
    
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    delay(500);
    udp.begin(WiFi.localIP(),udpPort);
    Serial.println("Now sending...");
}

void loop() {
  int photo = analogRead(PHOTO_PIN);
  int piezo = analogRead(PIEZO_PIN);
  sendMessage(photo, piezo);
  delay(50);
}
  
void sendMessage(int photo, int piezo) {
  udp.beginPacket(udpAddress,udpPort);
  udp.printf("%d %d", photo, piezo);
  udp.endPacket();
}
