#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "WiFiAdapter.h"

WiFiAdapter::WiFiAdapter() {}

bool WiFiAdapter::connectEnterprise(const char* user, const char* pass, const char* ssid) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.printf("\nConnecting to %s ...", ssid);
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)user, strlen(user)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)user, strlen(user)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)pass, strlen(pass)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable(&config) );
    WiFi.begin(ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());
    Serial.print("OK: ");
    Serial.println(WiFi.localIP());
    return true;
}

bool WiFiAdapter::connectPrivate(const char* pass, const char* ssid) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.printf("\nConnecting to %s ...", ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());
    Serial.print("OK: ");
    Serial.println(WiFi.localIP());
    return true;
}