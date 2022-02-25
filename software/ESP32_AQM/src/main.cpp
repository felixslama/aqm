#include <Arduino.h>
#include "web.h"
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  initWifi();
  initWeb();
}

void loop() {
  wifiCheck();
}