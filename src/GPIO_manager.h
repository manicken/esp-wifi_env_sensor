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
#elif defined(ESP32)
#include <WiFi.h>
#include <fs_WebServer.h>
#include "mimetable.h"

#include <mdns.h>
#endif

/*
    this file is only intended to manage which functions that are assigned to a specific GPIO pin
    that table is stored into a JSON file
    this will also contain the webserver functions, such as retrival of available GPIO pins

    in the client UI:
    GPIO pin selection should be from a predefined list of available GPIO:s
    so that GPIO:s that are assigned to fixed hardware functions such as
    I2C,UART,SPI,SD-card, etc.
    do not collide 
*/
namespace GPIO_manager
{
    #define GPIO_MANAGER_FILES_PATH                 F("/GPIO_manager")
    #define GPIO_MANAGER_CONFIG_JSON_FILE           F("/GPIO_manager/cfg.json")
    #define GPIO_MANAGER_GET_AVAILABLE_GPIO_LIST    F("/GPIO_manager/getAvailableGPIOs")

    enum class PinMode : int8_t {
        NA = -1,
        OUT = 0,
        IN = 1,
        IO = 2,
    };
    typedef struct {
        const char* Name;
        PinMode mode;
    } PinModeDef;

    const PinModeDef PinModeStrings[] = {
        {"NA", PinMode::NA},
        {"OUT", PinMode::OUT},
        {"IN", PinMode::IN},
        {"IO", PinMode::IO}
    };



    typedef struct {
        uint8_t pin;
        PinMode mode;
    } gpio_pin;
    

#ifdef ESP8266
    ESP8266WebServer *server = nullptr;
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
#endif
    
#if defined(ESP8266)
    const gpio_pin available_gpio_list[] {
        {0,PinMode::OUT},  // (reserved for programming) only safe to use as a output
        {1,PinMode::OUT},  // TXD0 (reserved for programming/UART) only safe to use as a output
        {2,PinMode::OUT},  // TXD1 (reserved for debug) only safe to use as a output
        //{3,PinMode::NA},  // RXD0 (reserved for programming/UART)
        {4,PinMode::IO},    // I2C SDA
        {5,PinMode::IO},    // I2C SCL
        //{6,PinMode::NA},  // SDCLK (reserved for spi flash)
        //{7,PinMode::NA},  // SDD0 (reserved for spi flash)
        //{8,PinMode::NA},  // SDD1 (reserved for spi flash)
        //{9,PinMode::NA},  // SDD2 (reserved for spi flash)
        //{10,PinMode::NA}, // SDD3 (reserved for spi flash)
        //{11,PinMode::NA}, // SDCMD (reserved for spi flash)
        {12,PinMode::IO}, // SPI MISO
        {13,PinMode::IO}, // SPI MOSI
        {14,PinMode::IO}, // SPI SCLK
        {15,PinMode::IO}, // SPI CS/TXD2
    };
#elif defined(ESP32)
    const gpio_pin available_gpio_list[] {
        //{0,PinMode::OUT}, // ADC2_1/TOUCH1 (reserved for programming)
        //{1,PinMode::OUT}, // U0_TXD (reserved for programmer/debug)
        {2,PinMode::OUT}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
        //{3,PinMode::NA}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
        {4,PinMode::IO},  // ADC2_0/TOUCH0/SD_DATA1
        {5,PinMode::OUT},  // VSPI_CS (must be HIGH during boot)
        //{6,PinMode::NA},  // U1_CTS/SPI_CLK (reserved for flash)
        //{7,PinMode::NA},  // U2_RTS/SPI_MISO (reserved for flash)
        //{8,PinMode::NA},  // U2_CTS/SPI_MOSI (reserved for flash)
        //{9,PinMode::NA},  // U1_RXD/SPI_HD (reserved for flash)
        //{10,PinMode::NA}, // U1_TXD/SPI_WP (reserved for flash)
        //{11,PinMode::NA}, // U1_RTX/SPI_CS (reserved for flash)
        {12,PinMode::OUT}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
        {13,PinMode::IO}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3
        {14,PinMode::IO}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK
        {15,PinMode::OUT}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
        {16,PinMode::IO}, // U2_RXD
        {17,PinMode::IO}, // U2_TXD
        {18,PinMode::IO}, // VSPI_CLK
        {19,PinMode::IO}, // VSPI_MISO
        {21,PinMode::IO}, // I2C_SDA
        {22,PinMode::IO}, // I2C_SCL
        {23,PinMode::IO}, // VSPI_MOSI
        {25,PinMode::IO}, // ADC2_8/DAC1
        {26,PinMode::IO}, // ADC2_9/DAC2
        {27,PinMode::IO}, // ADC2_7/TOUCH7
        {32,PinMode::IO}, // ADC1_4/TOUCH9/XTAL32
        {33,PinMode::IO}, // ADC1_5/TOUCH8/XTAL32
        {34,PinMode::IN}, // ADC1_6 (input only)
        {35,PinMode::IN}, // ADC1_7 (input only)
        {36,PinMode::IN}, // ADC1_0/SensVP (input only)
        {39,PinMode::IN}  // ADC1_3/SensVN (input only)
        };
#endif
    const uint8_t available_gpio_list_lenght = sizeof(available_gpio_list)/sizeof(available_gpio_list[0]);
    const uint8_t PinModeStrings_lenght = sizeof(PinModeStrings)/sizeof(PinModeStrings[0]);

    void sendList()
    {
        String srv_return_msg = "{";
#if defined(ESP8266)
        srv_return_msg.concat("\"MCU\":\"ESP8266\",");
#elif defined(ESP32)
        srv_return_msg.concat("\"MCU\":\"ESP32\",");
#endif
        srv_return_msg.concat("\"PinModes\":{");
        for (int i=0;i<PinModeStrings_lenght;i++)
        {
            /*
            srv_return_msg.concat("{\"mode\":");
            srv_return_msg.concat((int8_t)PinModeStrings[i].mode);
            srv_return_msg.concat(",\"str\":\"");
            srv_return_msg.concat(PinModeStrings[i].Name);
            srv_return_msg.concat("\"}");
            */
            srv_return_msg.concat("\"");
            srv_return_msg.concat((int8_t)PinModeStrings[i].mode);
            srv_return_msg.concat("\":\"");
            srv_return_msg.concat(PinModeStrings[i].Name);
            srv_return_msg.concat("\"");
            if (i<(PinModeStrings_lenght-1))
                srv_return_msg.concat(",");
        }
        srv_return_msg.concat("},");
        srv_return_msg.concat("\"list\":{");
        for (int i=0;i<available_gpio_list_lenght;i++)
        {
            /*srv_return_msg.concat("{\"pin\":");
            srv_return_msg.concat(available_gpio_list[i].pin);
            srv_return_msg.concat(",\"mode\":");
            srv_return_msg.concat((int8_t)available_gpio_list[i].mode);
            srv_return_msg.concat("}");*/
            srv_return_msg.concat("\"");
            srv_return_msg.concat(available_gpio_list[i].pin);
            srv_return_msg.concat("\":");
            srv_return_msg.concat((int8_t)available_gpio_list[i].mode);
            if (i<(available_gpio_list_lenght-1))
                srv_return_msg.concat(",");
        }
        srv_return_msg.concat("}");
        srv_return_msg.concat("}");
        server->send(200, "text/json", srv_return_msg);
    }
#if defined(ESP8266)
    void setup(ESP8266WebServer &srv) {
#elif defined(ESP32)
    void setup(fs_WebServer &srv) {
#endif
        server = &srv;
        srv.on(GPIO_MANAGER_GET_AVAILABLE_GPIO_LIST, HTTP_GET, sendList);
    }

    bool loadJson()
    {
        if (!LittleFS.exists(GPIO_MANAGER_FILES_PATH))
        {
            LittleFS.mkdir(GPIO_MANAGER_FILES_PATH);
            DEBUG_UART.println("GPIO manager ERROR - dir did not exist");
            return false;
        }

        if( LittleFS.exists(GPIO_MANAGER_CONFIG_JSON_FILE) == false) {
            DEBUG_UART.println("GPIO manager ERROR - cfg file did not exist");
            return false;
        }

        DynamicJsonDocument jsonDoc(256);
        char jsonBuffer[256];

        LittleFS_ext::load_from_file(GPIO_MANAGER_CONFIG_JSON_FILE, jsonBuffer);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            DEBUG_UART.print("GPIO manager ERROR - cfg Deserialization failed: ");
            DEBUG_UART.println(error.c_str());
            return false;
        }

        // Iterate through all key-value pairs in the JSON object
        for (JsonPair kv : jsonDoc.as<JsonObject>()) {
            const char* key = kv.key().c_str();  // Get the key as a string
            const char* type = kv.value().as<const char*>();  // Get the value as a string

            uint8_t pin = atoi(key);
            
            /*if (strncmp(type, "DHT", 3) == 0)
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
            }*/
        }
    }
}