#include <Arduino.h>
#include <BLEDevice.h>
#include "BLE.h"

// BLE Service Measurement UUID
static BLEUUID XENSIV_Measurement("2a13dada-295d-f7af-064f-28eac027639f");
// Characteristics UUIDs
static BLEUUID CO2_Data("4ef31e63-93b4-eca8-3846-84684719c484");
static BLEUUID Pressure_Data("0b4f4b0c-0795-1fab-a44d-ab5297a9d33b");
static BLEUUID Temperature_Data("7eb330af-8c43-f0ab-8e41-dc2adb4a3ce4");
static BLEUUID Humidity_Data("421da449-112f-44b6-4743-5c5a7e9c9a1f");
// BLE Service Config UUID
static BLEUUID XENSIV_Config("2119458a-f72c-269b-4d4d-2df0319121dd");
// Alarm Threshold UUID
static BLEUUID Alarm_Threshold("4ffb7e99-85ba-de86-4242-004f76f23409");
// Sample Rate UUID
static BLEUUID Sample_Rate("8420e6c6-49ba-7c8d-104f-10fe496d061f");
// Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};

BLECO2SenseNetServer::BLECO2SenseNetServer(const char* deviceName) {
    _deviceName = deviceName;
}

void BLECO2SenseNetServer::scan() {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(this);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
}

void BLECO2SenseNetServer::onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == _deviceName) {
        advertisedDevice.getScan()->stop();
        advertisedDevice.getScan()->clearResults();
        advertisedDevice.getScan()->setActiveScan(false);
        _serverAddress = new BLEAddress(advertisedDevice.getAddress());
        _found = true;
        Serial.println("BLE SenseNet found!");
    }
}

String BLECO2SenseNetServer::getAddressStr() {
    return String(_serverAddress->toString().c_str());
}

bool BLECO2SenseNetServer::found() {
    return _found;
}

bool BLECO2SenseNetServer::connected() {
    return _connected;
}

bool BLECO2SenseNetServer::connect(notify_callback co2NotifyCallback, 
                                        notify_callback temperatureNotifyCallback, 
                                        notify_callback pressureNotifyCallback, 
                                        notify_callback humidityNotifyCallback) {
    Serial.print("Creating client...");
    if (_bleClient == nullptr) {
        _bleClient = BLEDevice::createClient();
    }
    Serial.println("ok");
    Serial.print(String("Disconnecting from " + getAddressStr()));
    _bleClient->disconnect();
    Serial.println(" ... ok");
    // Connect to the remote BLE Server.
    Serial.print(String("Connnecting to " + getAddressStr()));
    while (!_bleClient->connect(*_serverAddress)){
        delay(200);
        Serial.print(".");    
    }
    Serial.println("ok");
    _connected = true;
    // Obtain a reference to the measurement service
    Serial.print("Connecting to measurement service...");
    free(_remoteMeasurementService);
    _remoteMeasurementService = _bleClient->getService(XENSIV_Measurement);
    while (_remoteMeasurementService == nullptr) {
        delay(500);
        Serial.print(".");
        _remoteMeasurementService = _bleClient->getService(XENSIV_Measurement);
    }
    Serial.println("connected");
    // Obtain references to the characteristics in the measurement service of the remote BLE server.
    _co2Characteristic = _remoteMeasurementService->getCharacteristic(CO2_Data);
    _temperatureCharacteristic = _remoteMeasurementService->getCharacteristic(Temperature_Data);
    _pressureCharacteristic = _remoteMeasurementService->getCharacteristic(Pressure_Data);
    _humidityCharacteristic = _remoteMeasurementService->getCharacteristic(Humidity_Data);
    if (_co2Characteristic == nullptr 
    || _temperatureCharacteristic == nullptr
    || _pressureCharacteristic == nullptr
    || _humidityCharacteristic == nullptr) {
        return false;
    }
    _co2NotifyCallback = co2NotifyCallback;
    _temperatureNotifyCallback = temperatureNotifyCallback;
    _pressureNotifyCallback = pressureNotifyCallback;
    _humidityNotifyCallback = humidityNotifyCallback;
    // Assign callback functions for the Characteristics
    _co2Characteristic->registerForNotify(co2NotifyCallback);
    _temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
    _pressureCharacteristic->registerForNotify(pressureNotifyCallback);
    _humidityCharacteristic->registerForNotify(humidityNotifyCallback);
    // Activate the Notify property of each Characteristic 
    _co2Characteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
    _temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
    _pressureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
    _humidityCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
    return true;
}

void BLECO2SenseNetServer::disconnect() {
    _connected = false;
}
