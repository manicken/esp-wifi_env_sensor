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
        String ToString()
        {
            String str = "";
            str.concat("uid="); str.concat(uid);
            str.concat(", pin="); str.concat(pin);
            return str;
        }
    };

    struct Device {
    protected:
        Device() {}
    public:
        uint32_t uid;
        DeviceType type;
        uint8_t pin;

        virtual String ToString() {
            String str;
            str.concat("uid="); str.concat(uid);
            str.concat(", type="); str.concat((int)type);
            str.concat(", pin="); str.concat(pin);
            return str;
        }
        
    };
    struct OneWireDevice : public Device {
    protected:
        OneWireDevice() {}
    public:
        uint8_t* romid;
        bool IsValid() { return romid != nullptr; }
        ~OneWireDevice();
    };
    struct OneWireTempDevice : public OneWireDevice {
        float value;
        
        OneWireTempDevice(uint32_t _uid, uint8_t _pin, const char* romid_hexstr)
        {
            romid = new uint8_t[8]();
            if (convertHexToBytes(romid_hexstr, romid, 8) == false) { delete[] romid; romid = nullptr;}
            // note later usage must allways check beforehand if romid is nullptr before use
            type = DeviceType::OneWireTemp;
            uid = _uid;
            pin = _pin;
            value = 0;
        }
        String ToString() override
        {
            String str = Device::ToString();
            str.concat("romid="); 
            if (romid != nullptr)
                str.concat(ByteArrayToString(romid, 8).c_str());
            else
                str.concat("nullptr");
            str.concat("value="); str.concat(value);
        }
    };
    struct DHTdevice : public Device {
        DHT_Type dhtType;
        float value;
        DHTdevice(const char* dhtTypeStr, uint32_t _uid, uint8_t _pin) {
            type = DeviceType::DHT;
            if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT11, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT11)-1) == 0)
                dhtType = DHT_Type::DHT11;
            else if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT22, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT22)-1) == 0)
                dhtType = DHT_Type::DHT22;
            else if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT_AM2302, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT_AM2302)-1) == 0)
                dhtType = DHT_Type::AM2302;
            else if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT_RHT03, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT_RHT03)-1) == 0)
                dhtType = DHT_Type::RHT03;
            else
                dhtType = DHT_Type::Unknown;

            uid = _uid;
            pin = _pin;
            value = 0;
        }
        String ToString() override
        {
            String str = Device::ToString();
            str.concat("dhtType="); str.concat((int)dhtType);
            str.concat("value="); str.concat(value);
        }
    };
    struct PWMdevice : public Device {
        float frequency;
        uint8_t bits;
        uint8_t invOut;
        PWMdevice(uint32_t _uid, uint8_t _pin, float _frequency, uint8_t _bits, uint8_t _invOut)
        {
            type = DeviceType::PWM;
            uid = _uid;
            pin = _pin;
            frequency = _frequency;
            bits = _bits;
            invOut = _invOut;
        }
        String ToString() override
        {
            String str = Device::ToString();
            
            str.concat("frequency="); str.concat(frequency);
            str.concat(", bits="); str.concat(bits);
            str.concat(", invOut="); str.concat(invOut);
            return str;
        }
    };
    struct TX433device : public Device {
        TX433device(uint32_t _uid, uint8_t _pin)
        {
            type = DeviceType::TX433;
            uid = _uid;
            pin = _pin;
        }
    };
    void NewStuctureTest()
    {
        Device *owtd_ptr = new OneWireTempDevice(0,0, "00:00:00:00:00:00:00:00");
        
        if (owtd_ptr->type == DeviceType::OneWireTemp) {
            OneWireTempDevice& owtd = static_cast<OneWireTempDevice&>(*owtd_ptr);
        }
        delete owtd_ptr;
    }
/*
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
    };*/
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
    int getTotalCountOfOneWireTempDevices();
    bool contains(int *array, size_t arraySize, uint8_t value);
    bool getAllOneWireTemperatures();
    void htmlGetAllOneWireTemperatures();
    bool getValue(uint32_t uid, float* value);
    void setValue(uint32_t uid, float value);
    void setValue(uint32_t uid, uint32_t value);
    void htmlGetListOfOneWireDevicesOnBusPin();
}
