
/*
this module could be called device manager
and manage all external connected devices

using the following json format:
here every key is the uid for that specific device,
so that it can be referred from other modules
such as thingsspeak

{
	"0":{type:"onewire",pin:25,romid:"2800000000000000",description:"temp ute"},
	"1":{type:"onewire",pin:25,romid:"2800000000000001",description:"temp inne"},
	"2":{type:"onewire",pin:26,romid:"2800000000000002",description:"temp jord"},
	"3":{type:"dht",pin:27,description:"fuktighet inne"},
	"4":{type:"dht",pin:18,description:"fuktighet ute"},
	"5":{type:"fan",pin:19,description:"främre takfläkt"},
    "6":{type:"rf433,pin:37,des"}
}
*/

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
#ifndef DEBUG_UART
#define DEBUG_UART Serial1
#endif
#elif defined(ESP32)
#include <WiFi.h>
#include <fs_WebServer.h>
#include "mimetable.h"

#include <mdns.h>
//#ifndef DEBUG_UART
#define DEBUG_UART Serial
//#endif
 
#endif

// sensors
#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// output devices
//#include "RF433.h"
//#include "FAN.h"

namespace DeviceManager
{
    #define DEVICE_MANAGER_FILES_PATH                 F("/DeviceManager")
    #define DEVICE_MANAGER_CONFIG_JSON_FILE           F("/DeviceManager/cfg.json")
    #define DEVICE_MANAGER_RELOAD_JSON                F("/DeviceManager/reloadJson")

    enum class DeviceType : int
    {
        Unknown = -1,
        OneWire = 0,
        DHT = 1,
        FAN = 2,
        RF433 = 3
    };

    struct Device {
        uint32_t uid;
        DeviceType type;
        uint8_t pin;
        char* romid; // only used for oneWire
        ~Device() {
            if (romid != nullptr)
                free(romid);
        }
    };

    Device *devices = nullptr;

#ifdef ESP8266
    ESP8266WebServer *server = nullptr;
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
#endif

    DHTesp dht;

    OneWire oneWire;
    DallasTemperature dTemp(&oneWire);



    void initDHT(uint8_t pin, const char * type)
    {
        type+=3;

        if (strncmp(type, "11", 2) == 0) {
            dht.setup(pin, DHTesp::DHT11);
        }
        else if (strncmp(type, "22", 2) == 0) {
            dht.setup(pin, DHTesp::DHT22);
        }
        else if (strncmp(type, "_AM2302", 7) == 0) {
            dht.setup(pin, DHTesp::AM2302);
        }
        else if (strncmp(type, "_RHT03", 6) == 0) {
            dht.setup(pin, DHTesp::RHT03);
        }
    }

    bool readJson()
    {
        DEBUG_UART.println("readJson func start");
        
        

        if (!LittleFS.exists(DEVICE_MANAGER_FILES_PATH))
        {
            LittleFS.mkdir(DEVICE_MANAGER_FILES_PATH);
            DEBUG_UART.println("Device Manager ERROR - dir did not exist");
            return false;
        }

        if( LittleFS.exists(DEVICE_MANAGER_CONFIG_JSON_FILE) == false) {
            DEBUG_UART.println("Device Manager ERROR - cfg file did not exist");
            return false;
        }
        DEBUG_UART.println("loading..");
        int size = LittleFS_ext::getFileSize(DEVICE_MANAGER_CONFIG_JSON_FILE);
        DEBUG_UART.print("File size:"); DEBUG_UART.println(size);

        char jsonBuffer[size+1]; // +1 for null char
        if (LittleFS_ext::load_from_file(DEVICE_MANAGER_CONFIG_JSON_FILE, jsonBuffer) == false)
        {
            DEBUG_UART.println("error could not load file:");
            DEBUG_UART.println(DEVICE_MANAGER_CONFIG_JSON_FILE);
            return false;
        }
        /*Serial.print("jsonBuffer: [");
        for (int i = 0;i< size; i++)
            Serial.print(jsonBuffer[i]);
        //Serial.print(jsonBuffer);
        Serial.println("]");*/
        DEBUG_UART.println("[OK]");

        DynamicJsonDocument jsonDoc(1024);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        DEBUG_UART.println("deserializing..");
        if (error)
        {
            DEBUG_UART.print("Device Manager ERROR - cfg Deserialization failed: ");
            DEBUG_UART.println(error.c_str());
            return false;
        }
        DEBUG_UART.println("[OK]");
        DEBUG_UART.print("item count:"); DEBUG_UART.println(jsonDoc.size());
        if (devices != nullptr) free(devices);
        devices = (Device*)malloc(sizeof(Device) * jsonDoc.size());
        int currIndex = 0;
        // Iterate through all key-value pairs in the JSON object
        for (JsonPair kv : jsonDoc.as<JsonObject>()) {
            Device device = devices[currIndex++];
            const char* uid = kv.key().c_str();  // Get the key as a string
            

            if (kv.value().is<JsonObject>() == false) {DEBUG_UART.println("item is not JSON object"); device.type = DeviceType::Unknown; continue;}
            JsonObject item = kv.value().as<JsonObject>();
            if (item.containsKey("type") == false){DEBUG_UART.println("item do not contain a type"); device.type = DeviceType::Unknown;  continue; }
            if (item.containsKey("pin") == false){DEBUG_UART.println("item do not contain a pin"); device.type = DeviceType::Unknown;  continue; }

            const char* type = item["type"].as<const char*>();
            if (item["pin"].is<int>() == false) {DEBUG_UART.println("pin is not a integer"); device.type = DeviceType::Unknown; continue;}

            device.uid = atoi(uid);
            device.pin = item["pin"].as<int>();
            if (strncmp(type, "onewire", 7) == 0) device.type = DeviceType::OneWire;
            else if (strncmp(type, "dht", 7) == 0) device.type = DeviceType::DHT;
            else if (strncmp(type, "fan", 7) == 0) device.type = DeviceType::FAN;
            else if (strncmp(type, "rf433", 7) == 0) device.type = DeviceType::RF433;
            else device.type = DeviceType::Unknown;

            
            //const char* pinStr = item["pin"].as<const char*>();
            //if (!pinStr) { DEBUG_UART.println("pin could not convert to a string"); continue;}
            
            DEBUG_UART.println();
            DEBUG_UART.print("Uid:"); DEBUG_UART.println(device.uid);
            DEBUG_UART.print("Type:"); DEBUG_UART.println((int)device.type);
            DEBUG_UART.print("Pin:"); DEBUG_UART.println(device.pin);
            if (item.containsKey("romid")) {
                const char* romid = item["romid"].as<const char*>(); 
                device.romid = strdup(romid);
                DEBUG_UART.print("RomId:"); DEBUG_UART.println(device.romid);
            }
            else device.romid = nullptr;
            // note. description is currently only used in the GUI editor
            if (item.containsKey("description")) {
                const char* descr = item["description"].as<const char*>(); 
                DEBUG_UART.print("Description:"); DEBUG_UART.println(descr);
            }
            
            /*
            if (strncmp(type, "DHT", 3) == 0)
                initDHT(pin, type);
            else if (strncmp(type, "ONE_WIRE", 8) == 0) {
                oneWire.begin(pin);
                dTemp.setOneWire(&oneWire);
                dTemp.begin();
            }
            else if (strncmp(type, "FAN", 3) == 0) {
                FAN::pin = pin;
                FAN::init();
            }
            else if (strncmp(type, "RF433", 5) == 0) { // TODO rename all RF433 to TX433
                RF433::init(pin);
            }
            */
        }
        return true;
    }

    void reloadJSON();

#ifdef ESP8266
    void setup(ESP8266WebServer &srv) {
#elif defined(ESP32)
    void setup(fs_WebServer &srv) {
#endif
        server = &srv;
        srv.on(DEVICE_MANAGER_RELOAD_JSON, HTTP_GET, reloadJSON);
    }

    void reloadJSON()
    {
        // use HTTP/1.1 Chunked response to avoid building a huge temporary string
        if (!server->chunkedResponseModeStart(200, "text/html")) {
            server->send(505, F("text/html"), F("HTTP1.1 required"));
            return;
        }
        server->sendContent("LOAD JSON start<br>");
        if (readJson())
            server->sendContent("LOAD JSON ok<br>");
        else
            server->sendContent("LOAD JSON fail<br>");
        server->chunkedResponseFinalize();
    }
}