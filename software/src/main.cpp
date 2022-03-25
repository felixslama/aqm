#include <Arduino.h>
#include "CO2BLE.h"
#include <ArduinoJson.h>
#include "WiFiAdapter.h"
#include "MQTT.h"
#include "Credentials.h"
#include "Utility.h"
#include "Web.h"

// Variables to store the sansor values
int co2Value;
double temperatureValue;
double pressureValue;
double humidityValue;

int sampleRateValue;
int alarmThresholdValue;

int failCounter = 0;
const int MAX_FAILS = 5;

// Flag to check whether new readings are available
boolean newCo2Value = false;
boolean newTemperatureValue = false;
boolean newPressureValue = false;
boolean newHumidityValue = false;

boolean newAlarmThresholdValue = false;
boolean newSampleRateValue = false;

BLECO2SenseNetServer* bleServer = nullptr;
WiFiAdapter* wifi = nullptr;
MQTTClient* mqttClient = nullptr;

boolean enterpriseWifi = false;

String boardAddress = "";
int mqttPort = 8883;

// When the BLE Server sends a new CO2 reading with the notify property
static void co2NotifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // store the CO2 value
  co2Value = ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  newCo2Value = true;
}

// When the BLE Server sends a new temerature reading with the notify property
static void temperatureNotifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  int temp_low = ((pData[3] & 0xFF) << 8) + (pData[2] & 0xFF);
  int temp_high = ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  
  temperatureValue = temp_high + (double)temp_low / 1000;
  temperatureValue -= 4; // approx. correction of on-board self-heating
  newTemperatureValue = true;
}

// When the BLE Server sends a new pressure reading with the notify property
static void pressureNotifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  int pressure = ((pData[3] & 0xFF) << 24) + ((pData[2] & 0xFF) << 16) + ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  pressureValue = (double)pressure / 100;
  newPressureValue = true;
}

// When the BLE Server sends a new humidity reading with the notify property
static void humidityNotifyCallback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, 
                              uint8_t* pData, size_t length, bool isNotify) {
  int hum_low = ((pData[3] & 0xFF) << 8) + (pData[2] & 0xFF);
  int hum_high = ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  
  humidityValue = hum_high + (double)hum_low / 1000;                               
  newHumidityValue = true;
}

// When the MQTT Server sends a new message to the subscribed topic
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  wifi = new WiFiAdapter();
  boolean connectedToWifi = false;
  if (enterpriseWifi) {
    connectedToWifi = wifi->connectEnterprise(userEnterprise, passEnterprise, ssidEnterprise);
  }
  else {
    connectedToWifi = wifi->connectPrivate(passPrivate, ssidPrivate);
  }
  
  if (connectedToWifi) {
    Serial.println("WiFi connected");
    setDateTime();
    Serial.println("Local Time synchronized");
    Serial.println(getDateTimeStr());
  }

  if (connectedToWifi) {
    Serial.println("Starting HTTP Server");
    initWeb();
    Serial.println("HTTP server started");
    Serial.println("Connecting MQTT Broker");
    mqttClient = new MQTTClient(boardAddress.c_str(), mqttServer, mqttPort, mqttUsername, mqttPassword, mqttCallback);
    if (mqttClient->connect()) {
      Serial.println("MQTT Broker connected");
    }
  }
}

void loop() {

  if (bleServer == nullptr) {
    bleServer = new BLECO2SenseNetServer(senseNetName);
    bleServer->scan();
    Serial.println("Scanning initiated.");
  }

  if (!bleServer->found()) {
    while (!bleServer->found()) {
      delay(500);
    }
    boardAddress = bleServer->getAddressStr();
  }

  if (!bleServer->connected()) {
    Serial.println("Connecting callbacks...");
    while (!bleServer->connect(co2NotifyCallback, temperatureNotifyCallback, pressureNotifyCallback, humidityNotifyCallback)) {
      delay(500);
      Serial.println("Waiting for callbacks");
    }
    boardAddress = bleServer->getAddressStr();
  }

  Serial.print("\nWaiting for sensor data ");
  int counter = 0;
  while (!newCo2Value && !newTemperatureValue && !newPressureValue && !newHumidityValue) {
    Serial.print(".");
    counter++;
    delay(200);
    if (counter == 20) {
      failCounter++;
      break;
    }
  }
  newCo2Value = false;
  newTemperatureValue = false;
  newPressureValue = false;
  newHumidityValue = false;
  
  if (failCounter == 0) 
  {
    Serial.println("ok\n");
    
    StaticJsonDocument<1024> doc;
    doc[String("Address")] = boardAddress;
    doc[String("Timestamp")] = getDateTimeStr();

    doc[String("CO2")] = co2Value;

    Serial.println();
    doc[String("Temperature")] = temperatureValue;

    doc[String("Pressure")] = pressureValue;

    doc[String("Humidity")] = humidityValue;

    doc[String("IP")] = WiFi.localIP();

    String payload = String();
    serializeJson(doc, payload);
    Serial.printf("\nPayload: %s\n", payload.c_str());

    Serial.print("Sending payload...");
    bool ok = mqttClient->publish("ble_sensor_values", payload.c_str());
    Serial.printf("%s\n", ok? "ok":"failed");
    if (!ok) {
      failCounter++;   
    }  
  }
  else {
    bleServer->disconnect();
  }
  delay(500);
}
