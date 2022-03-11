#include <Arduino.h>

class WiFiAdapter {
    public:
        WiFiAdapter();
        bool connectEnterprise(const char* user, const char* pass, const char* ssid);
        bool connectPrivate(const char* pass, const char* ssid);
};