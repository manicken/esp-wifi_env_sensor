#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"

// sensors
#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// output devices
#include "RF433.h"
#include "FAN.h"

#define DEBUG_UART Serial1

namespace ExtPeripherals
{
    #define EXT_PERIPHERALS_FILES_PATH                 F("/ExtPeripherals")
    #define EXT_PERIPHERALS_CONFIG_JSON_FILE           F("/ExtPeripherals/cfg.json")

    DHTesp dht;

    OneWire oneWire(ONE_WIRE_BUS);
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

    bool init()
    {
        DynamicJsonDocument jsonDoc(256);
        char jsonBuffer[256];

        if (!LittleFS.exists(EXT_PERIPHERALS_FILES_PATH))
        {
            LittleFS.mkdir(EXT_PERIPHERALS_FILES_PATH);
            DEBUG_UART.println("Ext Peripherals ERROR - dir did not exist");
            return false;
        }

        if( LittleFS.exists(EXT_PERIPHERALS_CONFIG_JSON_FILE) == false) {
            DEBUG_UART.println("Ext Peripherals ERROR - cfg file did not exist");
            return false;
        }

        LittleFS_ext::load_from_file(EXT_PERIPHERALS_CONFIG_JSON_FILE, jsonBuffer);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            DEBUG_UART.print("Ext Peripherals ERROR - cfg Deserialization failed: ");
            DEBUG_UART.println(error.c_str());
            return false;
        }

        // Iterate through all key-value pairs in the JSON object
        for (JsonPair kv : jsonDoc.as<JsonObject>()) {
            const char* key = kv.key().c_str();  // Get the key as a string
            const char* type = kv.value().as<const char*>();  // Get the value as a string

            uint8_t pin = atoi(key);
            
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
        }

    }
}