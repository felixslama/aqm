#include <BLEAdvertisedDevice.h>

class BLECO2SenseNetServer: public BLEAdvertisedDeviceCallbacks {
    private:
        const char* _deviceName;
        bool _found = false;
        bool _connected = false;

        // Address of the peripheral device. Address will be found during scanning...
        BLEAddress *_serverAddress = nullptr;
        BLEClient *_bleClient = nullptr;
        BLERemoteService* _remoteMeasurementService = nullptr;

        // callbacks
        notify_callback _co2NotifyCallback; 
        notify_callback _temperatureNotifyCallback;
        notify_callback _pressureNotifyCallback;
        notify_callback _humidityNotifyCallback;

        // Characteristics that we want to read
        BLERemoteCharacteristic* _co2Characteristic;
        BLERemoteCharacteristic* _pressureCharacteristic;
        BLERemoteCharacteristic* _temperatureCharacteristic;
        BLERemoteCharacteristic* _humidityCharacteristic;

    public:
        BLECO2SenseNetServer(const char* deviceName);

        void scan();

        void onResult(BLEAdvertisedDevice advertisedDevice);

        String getAddressStr();

        bool found();

        bool connect(notify_callback co2NotifyCallback, 
                     notify_callback temperatureNotifyCallback, 
                     notify_callback pressureNotifyCallback, 
                     notify_callback humidityNotifyCallback);

        bool connected();

        void disconnect();
};