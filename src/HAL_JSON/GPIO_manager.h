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

    void sendList();
    void setup(WEBSERVER_TYPE &srv);
    HAL_JSON_VERIFY_JSON_RETURN_TYPE CheckIfPinAvailable(uint32_t pin, PinMode mode);
    void ClearAllReservations();
    /** CheckIfPinAvailable must be called prior to using this function. */
    void ReservePin(uint8_t pin);
}