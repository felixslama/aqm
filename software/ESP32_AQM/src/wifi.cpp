#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid     = "aqm";
const char* password = "TSMCTSMC";
long previousWifiMillis = 0;
long intervalWifi = 500;

void initWifi(){
    WiFi.begin(ssid,password);
  
    while(WiFi.status() != WL_CONNECTED){
        Serial.println("Connecting to AQM-Wifi");
    }
}
void wifiCheck(){
    unsigned long currentWifiMillis = millis();
    if(currentWifiMillis - previousWifiMillis > intervalWifi) {
        previousWifiMillis = currentWifiMillis;
        Serial.println(String(WiFi.status()));
    }
}