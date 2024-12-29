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
    #define DEVICE_MANAGER_JSON_NAME_DHT_TYPE           "dht"
    #define DEVICE_MANAGER_JSON_NAME_PIN                "pin"
    #define DEVICE_MANAGER_JSON_NAME_BUS                "bus"
    #define DEVICE_MANAGER_JSON_NAME_UID                "uid" // device item uid
    #define DEVICE_MANAGER_JSON_NAME_ROMID              "romid" // used for one-wire devices
    #define DEVICE_MANAGER_JSON_NAME_DESCRIPTION        "note"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS  "1WB"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_TEMP "1WT"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT           "DHT"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT11         "DHT11"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT22         "DHT22"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT_AM2302    "AM2302"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DHT_RHT03     "RTH03"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_PWM           "PWM" // used for fan:s, servos, etc.
    #define DEVICE_MANAGER_JSON_NAME_TYPE_TX433         "TX433" // transmitter on RF @ 433MHz
    #define DEVICE_MANAGER_JSON_NAME_TYPE_NEO_PIXEL     "NEOPIX"
    // future types
    #define DEVICE_MANAGER_JSON_NAME_TYPE_ADC           "ADC"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DAC           "DAC"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DIN           "DIN"
    #define DEVICE_MANAGER_JSON_NAME_TYPE_DOUT          "DOUT"

    #define DEVICE_MANAGER_JSON_NAME_PWM_FREQ           "freq"
    #define DEVICE_MANAGER_JSON_NAME_PWM_BITS           "bits"
    #define DEVICE_MANAGER_JSON_NAME_PWM_FREQ           "freq"
    #define DEVICE_MANAGER_JSON_NAME_PWM_INV_OUT        "invOut"

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
        Unknown = -1,
        DHT11 = 0x11,
        DHT22 = 0x22,
        AM2302 = 0x2302,
        RHT03 = 0x3,
    };

    struct OneWireBus
    {
        uint32_t uid;
        uint8_t pin;
        String ToString();
    };

    struct Device {
      protected:
        Device() {}
      public:
        uint32_t uid;
        DeviceType type;
        uint8_t pin;

        virtual String ToString();
    };
    
    struct OneWireDevice : public Device {
      protected:
        OneWireDevice() {}
      public:
        uint8_t* romid;
        bool IsValid();
        ~OneWireDevice();
    };

    struct OneWireTempDevice : public OneWireDevice {
        float value;
        
        OneWireTempDevice(uint32_t _uid, uint8_t _pin, const char* romid_hexstr);
        String ToString() override;
    };

    struct DHTdevice : public Device {
        DHT_Type dhtType;
        float value;
        DHTdevice(const char* dhtTypeStr, uint32_t _uid, uint8_t _pin);
        String ToString() override;
    };

    struct PWMdevice : public Device {
        float frequency;
        uint8_t bits;
        uint8_t invOut;
        PWMdevice(uint32_t _uid, uint8_t _pin, float _frequency, uint8_t _bits, uint8_t _invOut);
        String ToString() override;
    };

    struct TX433device : public Device {
        TX433device(uint32_t _uid, uint8_t _pin);
    };

    enum class NEOPIXEL_Type {
        WS2811 = 0x2811,
        WS2812 = 0x2812,

    };
    struct NEOPIXELdevice : public Device {
        NEOPIXEL_Type subType;
        NEOPIXELdevice(uint32_t _uid, uint8_t _pin, NEOPIXEL_Type _subType);
    };

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize);
    bool convertHexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize);

#ifdef ESP8266
    void setup(ESP8266WebServer &srv);
#elif defined(ESP32)
    void setup(fs_WebServer &srv);
#endif

    // JSON helpers
    bool isValid_JsonOneWireBus_Item(JsonVariant jsonItem);
    bool isValid_JsonDevice_Item(JsonVariant jsonItem, const char*& type, uint32_t *uid = nullptr);
    bool isValid_JsonOneWireTemp_Item(JsonVariant jsonItem, const char*& romid, uint32_t *bus);
    bool isValid_JsonDHT_Item(JsonVariant jsonItem, const char*& dhtType);

    bool readJson();
    void reloadJSON();

    Device* getDeviceInfo(uint32_t uid);
    int getTotalCountOfOneWireTempDevices();
    bool contains(int *array, size_t arraySize, uint8_t value);
    bool getAllOneWireTemperatures();
    void htmlGetAllOneWireTemperatures();
    bool getValue(uint32_t uid, float* value);
    void setValue(uint32_t uid, float value);
    void setValue(uint32_t uid, uint32_t value);
    void htmlGetListOfOneWireDevicesOnBusPin();
}
