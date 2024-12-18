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

    #define DEVICE_MANAGER_JSON_NAME_TYPE               "type"
    #define DEVICE_MANAGER_JSON_NAME_PIN                "pin"
    #define DEVICE_MANAGER_JSON_NAME_BUS                "bus"
    #define DEVICE_MANAGER_JSON_NAME_UID                "uid" // device item uid
    #define DEVICE_MANAGER_JSON_NAME_ROMID              "romid" // used for one-wire devices
    #define DEVICE_MANAGER_JSON_NAME_DESCRIPTION        "note"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS  "1WB"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_TEMP "1WT"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT11         "DHT11"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT22         "DHT22"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT_AM2302    "AM2302"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT_RHT03     "RTH03"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_PWM           "PWM" // used for fan:s, servos, etc.
    #define DEVICE_MANAGER_JSON_NAME_TYPE_TX433         "TX433" // transmitter on RF @ 433MHz
    // future types
    #define DEVICE_MANAGER_JSON_NAME_TYPE_ADC           "ADC"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DAC           "DAC"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DIN           "DIN"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DOUT          "DOUT"

    enum class DeviceType : int32_t
    {
        Unknown = -1, //static_cast<int>(0xFFFFFFFF),
        OneWireBus = 0x1B,
        OneWireTemp = 0x10,
        DHT = 0x444854, // ascii hex for DHT
        PWM = 0xF,
        TX433 = 0x433,
        ADC = 0xA1, 
        DAC = 0xA0, // future ???
        DIN = 0xD1, // digital input
        DOUT = 0xD0 // digital output
    };

    enum class DHT_Type: int32_t
    {
        DHT11 = 0x11,
        DHT22 = 0x22,
        DHT_AM2302 = 0x2302,
        DHT_RHT03 = 0x3,
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
        DHT_Type dhtType;
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
