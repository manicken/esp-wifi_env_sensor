#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"

#include <WiFiClient.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#define WEBSERVER_TYPE ESP8266WebServer
#elif defined(ESP32)
#include <fs_WebServer.h>
#define WEBSERVER_TYPE fs_WebServer
#endif

#include "../Support/Logger.h"

#include "HAL_JSON_DeviceTypeDefNames.h"

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
    #define GPIO_MANAGER_ROOT                 "/GPIO_manager"
    #define GPIO_MANAGER_GET_AVAILABLE_GPIO_LIST    F(GPIO_MANAGER_ROOT "/getAvailableGPIOs")

    enum class PinMode : uint8_t {
        Reserved = 0x01,
        SpecialAtBoot = 0x02,
        LOW2BOOT = 0x04,
        HIGH2BOOT = 0x08,        
        OUT = 0x10,
        IN = 0x20,
        AIN = 0x40,
        AOUT = 0x80
    };

    typedef struct {
        const char* Name;
        uint8_t mode;
    } PinModeDef;

    enum class PrintListMode {
        String,
        Hex,
        Binary
    };
    // Templated makePinMask function
    /*template<typename... Modes>
    constexpr uint8_t makePinMask(Modes... modes) {
        return (static_cast<uint8_t>(modes) | ...);  // C++17 fold expression
    }*/

    typedef struct {
        uint8_t pin;
        uint8_t mode;
    } gpio_pin;

    void sendList();
    void setup(WEBSERVER_TYPE &srv);
    bool CheckIfPinAvailable(uint32_t pin, uint8_t pinMode);
    void ClearAllReservations();
    /** CheckIfPinAvailable must be called prior to using this function. */
    void ReservePin(uint8_t pin);
}