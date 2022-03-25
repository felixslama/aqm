#include <Arduino.h>
#include <ArduinoJson.h>
#include "BLE.h"
#include "WiFiAdapter.h"
#include "MQTT.h"
#include "Credentials.h"
#include "Utility.h"
#include "Web.h"

// Variables
int co2Value;
double temperatureValue;
double pressureValue;
double humidityValue;
int sampleRateValue;
int alarmThresholdValue;
int failCounter = 0;
int mqttInitCounter = 0;
String boardAddress = "";
int mqttPort = 8883;

// Bools
boolean newCo2Value = false;
boolean newTemperatureValue = false;
boolean newPressureValue = false;
boolean newHumidityValue = false;
boolean newAlarmThresholdValue = false;
boolean newSampleRateValue = false;
boolean enterpriseWifi = false;
boolean connectedToWifi = false;

// Classes
BLECO2SenseNetServer* bleServer = new BLECO2SenseNetServer(senseNetName);
WiFiAdapter* wifi = new WiFiAdapter();
MQTTClient* mqttClient = nullptr;

// When the BLE Server sends a new CO2 reading with the notify property
static void co2NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // store the CO2 value
  co2Value = ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  newCo2Value = true;
}

// When the BLE Server sends a new temerature reading with the notify property
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  int temp_low = ((pData[3] & 0xFF) << 8) + (pData[2] & 0xFF);
  int temp_high = ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  
  temperatureValue = temp_high + (double)temp_low / 1000;
  temperatureValue -= 4; // approx. correction of on-board self-heating
  newTemperatureValue = true;
}

// When the BLE Server sends a new pressure reading with the notify property
static void pressureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  int pressure = ((pData[3] & 0xFF) << 24) + ((pData[2] & 0xFF) << 16) + ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  pressureValue = (double)pressure / 100;
  newPressureValue = true;
}

// When the BLE Server sends a new humidity reading with the notify property
static void humidityNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                              uint8_t* pData, size_t length, bool isNotify) {
  int hum_low = ((pData[3] & 0xFF) << 8) + (pData[2] & 0xFF);
  int hum_high = ((pData[1] & 0xFF) << 8) + (pData[0] & 0xFF);
  
  humidityValue = hum_high + (double)hum_low / 1000;                               
  newHumidityValue = true;
}

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
  Serial.println("Free Heap: "+String(ESP.getFreeHeap()));
  Serial.println("Max Allocated Heap: "+String(ESP.getMaxAllocHeap()));
  Serial.println("Min Free Heap: "+String(ESP.getMinFreeHeap()));
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
  }
  // start scanning
  bleServer->scan();
}

void initMQTT() {
  Serial.println("Connecting MQTT Broker");
  mqttClient = new MQTTClient(boardAddress.c_str(), mqttServer, mqttPort, mqttUsername, mqttPassword, mqttCallback);
  if (mqttClient->connect()) {
    Serial.println("MQTT Broker connected");
  }
}

void loop() {
  if (mqttInitCounter == 4) {
    initMQTT();
  }

  if (!bleServer->found()) {
    while (!bleServer->found()) {
      delay(500);
    }
  }

  if (!bleServer->connected()) {
    Serial.println("Connecting...");
    bleServer->connect(co2NotifyCallback, temperatureNotifyCallback, pressureNotifyCallback, humidityNotifyCallback);
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
  
  if (failCounter == 0) {
    Serial.println("ok\n");
    StaticJsonDocument<1024> doc;
    doc[String("Address")] = boardAddress;
    doc[String("Timestamp")] = getDateTimeStr();
    doc[String("CO2")] = co2Value;
    doc[String("Temperature")] = temperatureValue;
    doc[String("Pressure")] = pressureValue;
    doc[String("Humidity")] = humidityValue;
    doc[String("IP")] = WiFi.localIP();
    String payload = String();
    serializeJson(doc, payload);
    Serial.printf("\nPayload: %s\n", payload.c_str());
    if (mqttInitCounter >= 4) {
      Serial.print("Sending payload...");
      bool ok = mqttClient->publish("ble_sensor_values", payload.c_str());
      Serial.printf("%s\n", ok? "ok":"failed");
    }
  } else {
    bleServer->disconnect();
  }
  mqttInitCounter++;
  delay(500);
}
