#include <NimBLEAdvertisedDevice.h>
#include <NimBLERemoteCharacteristic.h>

class BLECO2SenseNetServer: public NimBLEAdvertisedDeviceCallbacks 
{
    private:
        const char* _deviceName;
        bool _found = false;
        bool _connected = false;
        // Address of the peripheral device. Address will be found during scanning...
        NimBLEAddress *_serverAddress = nullptr;
        NimBLEClient *_bleClient = nullptr;
        NimBLERemoteService* _remoteMeasurementService = nullptr;
        // callbacks
        notify_callback _co2NotifyCallback; 
        notify_callback _temperatureNotifyCallback;
        notify_callback _pressureNotifyCallback;
        notify_callback _humidityNotifyCallback;
        // Characteristics that we want to read
        NimBLERemoteCharacteristic* _co2Characteristic;
        NimBLERemoteCharacteristic* _pressureCharacteristic;
        NimBLERemoteCharacteristic* _temperatureCharacteristic;
        NimBLERemoteCharacteristic* _humidityCharacteristic;
    public:
        BLECO2SenseNetServer(const char* deviceName);
        void scan();
        void onResult(NimBLEAdvertisedDevice* advertisedDevice);
        String getAddressStr();
        bool found();
        bool connect(notify_callback co2NotifyCallback, 
                     notify_callback temperatureNotifyCallback, 
                     notify_callback pressureNotifyCallback, 
                     notify_callback humidityNotifyCallback);
        bool connected();
        void disconnect();
};
