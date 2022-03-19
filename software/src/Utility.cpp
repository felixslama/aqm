#include <time.h>
#include <NTPClient.h>
#include "Utility.h"
#include <Esp32MQTTClient.h>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

void setDateTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

String getDateTimeStr() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return String("Failed to obtain time");
  }
  char buf[100];
  strftime (buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  String timestamp = buf;
  Serial.print("timestamp ");
  Serial.println(timestamp);
  return timestamp;
}

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}