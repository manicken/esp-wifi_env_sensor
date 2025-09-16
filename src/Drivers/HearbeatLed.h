#pragma once

#include <Arduino.h>

#if defined(ESP8266)
//#include <ESP8266WebServer.h>
#include <ESPAsyncWebServer.h>
#define DEBUG_UART Serial1
#define WEBSERVER_TYPE AsyncWebServer
#elif defined(ESP32)
//#include "Support/fs_WebServer.h"
#define DEBUG_UART Serial
//#define WEBSERVER_TYPE fs_WebServer
#include <ESPAsyncWebServer.h>
#define WEBSERVER_TYPE AsyncWebServer
#endif

#include "../Support/ConvertHelper.h"

namespace HeartbeatLed
{

#if defined (ESP32WROVER_E_IE)
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#elif defined (ESP32DEV)
    #define HEARTBEATLED_PIN 2
    #define HEARTBEATLED_ACTIVESTATE HIGH
    #define HEARTBEATLED_INACTIVESTATE LOW
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#elif defined (ESP8266)
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#endif

    extern unsigned long HEARTBEATLED_ON_INTERVAL;
    extern unsigned long HEARTBEATLED_OFF_INTERVAL;

    void setup(WEBSERVER_TYPE &srv);
    void task(void);
    
}