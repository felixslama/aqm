#include <WiFi.h>
#include "wifiConfig.h"

long previousWifiMillis = 0;
long intervalWifi = 500;

void initWifi(){
    WiFi.begin(ssid,password);
    WiFi.mode(WIFI_STA);
    while(WiFi.status() != WL_CONNECTED){
        Serial.println("Connecting to AQM-Wifi");
    }
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());
}
void wifiCheck(){
    unsigned long currentWifiMillis = millis();
    if(currentWifiMillis - previousWifiMillis > intervalWifi) {
        previousWifiMillis = currentWifiMillis;
        Serial.println(String(WiFi.status()));
    }
}