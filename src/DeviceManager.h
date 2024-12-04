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

    #define DEVICE_MANAGER_FILES_PATH                 F("/DeviceManager")
    #define DEVICE_MANAGER_CONFIG_JSON_FILE           F("/DeviceManager/cfg.json")
    #define DEVICE_MANAGER_URL_RELOAD_JSON            F("/DeviceManager/reloadJson")
    #define DEVICE_MANAGER_URL_GET_VALUE              F("/DeviceManager/getValue")
    #define DEVICE_MANAGER_URL_GET_ALL_1WIRE_TEMPS    F("/DeviceManager/getAll1wireTemps")

    enum class DeviceType : int
    {
        Unknown = -1,
        OneWire = 1,
        DHT11 = 11,
        DHT22 = 22,
        DHT_AM2302 = 2302,
        DHT_RHT03 = 3,
        FAN = 0,
        RF433 = 433
    };

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
    bool contains(uint8_t *array, size_t arraySize, uint8_t value);
    bool getAllOneWireTemperatures();
    void htmlGetAllOneWireTemperatures();
    bool getValue(uint32_t uid, float* value, uint8_t* resolution);
    void setValue(uint32_t uid, float value);
    void setValue(uint32_t uid, uint32_t value);
}
