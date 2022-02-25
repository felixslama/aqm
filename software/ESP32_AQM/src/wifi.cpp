#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid     = "aqm";
const char* password = "TSMCTSMC";

void initWifi(){
    WiFi.begin(ssid,password);
  
    while(WiFi.status() != WL_CONNECTED){
        Serial.println("Connecting to AQM-Wifi");
    }
}
void wifiCheck(){
    Serial.println(String(WiFi.status()));
    delay(500);
}