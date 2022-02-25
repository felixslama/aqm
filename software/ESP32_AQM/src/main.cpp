#include <Arduino.h>
#include "web.h"
#include "wifi.h"

void setup() {
  initWeb();
  initWifi();
}

void loop() {
  wifiCheck();
}