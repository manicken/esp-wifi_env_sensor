#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"

#include <WiFiClient.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#define DEBUG_UART Serial1
#elif defined(ESP32)
#include <WiFi.h>
#include <fs_WebServer.h>
#include "mimetable.h"
#include <mdns.h>
#define DEBUG_UART Serial
#endif

#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "GPIO_manager.h"

namespace DeviceManager
{
    #define BUSSES_DEV_PRINT

    #define DEVICE_MANAGER_FILES_PATH                  F("/DeviceManager")
    #define DEVICE_MANAGER_CONFIG_JSON_FILE            F("/DeviceManager/cfg.json")
    #define DEVICE_MANAGER_URL_RELOAD_JSON             F("/DeviceManager/reloadJson")
    #define DEVICE_MANAGER_URL_LIST_ALL_1WIRE_DEVICES  F("/DeviceManager/listAll1wireDevices")
    #define DEVICE_MANAGER_URL_GET_VALUE               F("/DeviceManager/getValue")
    #define DEVICE_MANAGER_URL_LIST_ALL_1WIRE_TEMPS    F("/DeviceManager/getAll1wireTemps")
    #define DEVICE_MANAGER_URL_PRINT_DEVICES           F("/DeviceManager/printDevices")

    enum class DeviceType : int32_t
    {
        Unknown = -1, //static_cast<int>(0xFFFFFFFF),
        OneWireBus = 0x1B,
        OneWireTemp = 0x10,
        DHT11 = 0x11,
        DHT22 = 0x22,
        DHT_AM2302 = 0x2302,
        DHT_RHT03 = 0x3,
        FAN = 0xF,
        TX433 = 0x433
    };

    // proposed new structure
    struct BaseDevice {
        uint32_t uid;
        DeviceType type;
        uint8_t pin;
    };
    struct OneWireDevice : public BaseDevice {
    protected:
        uint8_t* romid;
        OneWireDevice() {}
    public:
        ~OneWireDevice();
    };
    struct OneWireTempDevice : public OneWireDevice {
        float value;
    };
    struct DHTdevice : public BaseDevice {
        float value;
    };
    struct FANdevice : public BaseDevice {
        float frequency;
        uint8_t bits;
        uint8_t invOut;
    };
    struct TX433device : public BaseDevice {

    };
    void NewStuctureTest()
    {
        BaseDevice *owtd_ptr = new OneWireTempDevice();
        if (owtd_ptr->type == DeviceType::OneWireTemp) {
            OneWireTempDevice& owtd = static_cast<OneWireTempDevice&>(*owtd_ptr);
        }
        delete owtd_ptr;

        /// check https://chatgpt.com/c/6761deb2-23d4-800b-b837-347563ee25a9
    }
    // end of proposed new structure

    struct Device {
        uint32_t uid;
        DeviceType type;
        uint8_t pin;
        uint8_t* romid; // only used for oneWire
        union {
            float fvalue;
            uint32_t uintvalue;
            int32_t intvalue;
        };
        ~Device();
    };
    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize);
    bool convertHexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize);

#ifdef ESP8266
    void setup(ESP8266WebServer &srv);
#elif defined(ESP32)
    void setup(fs_WebServer &srv);
#endif

    bool readJson();
    void reloadJSON();

    Device* getDeviceInfo(uint32_t uid);
    int getTotalCountOfOneWireDevices();
    bool contains(int *array, size_t arraySize, uint8_t value);
    bool getAllOneWireTemperatures();
    void htmlGetAllOneWireTemperatures();
    bool getValue(uint32_t uid, float* value);
    void setValue(uint32_t uid, float value);
    void setValue(uint32_t uid, uint32_t value);
    void htmlGetListOfOneWireDevicesOnBusPin();
}
