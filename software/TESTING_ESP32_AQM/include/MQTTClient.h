#include "PubSubClient.h"
#include <WiFiClientSecure.h>


class MQTTClient {
    private:
        const char* _clientId;
        const char* _mqttServer;
        int _mqttPort;
        const char* _mqttUsername;
        const char* _mqttPassword;
        
    public:
        MQTTClient(const char* clientId, const char* mqttServer, int mqttPort, 
                   const char* mqttUsername, const char* mqttPassword, 
                   std::function<void (char *, uint8_t *, unsigned int)> callback);

        bool connect();
        void reconnect();
        bool publish(const char* topic, const char* payload);
        bool subscribe(const char* topic);
};