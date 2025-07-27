#pragma once


#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "Support/LittleFS_ext.h"

#include <WiFiClient.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#define WEBSERVER_TYPE ESP8266WebServer
#elif defined(ESP32)
#include "../Support/fs_WebServer.h"
#define WEBSERVER_TYPE fs_WebServer
#endif

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_GPIO_Manager.h"

#define HAL_JSON_GPIO_MANAGER_ROOT                 "/GPIO_manager"
#define HAL_JSON_GPIO_MANAGER_GET_AVAILABLE_GPIO_LIST    F(HAL_JSON_GPIO_MANAGER_ROOT "/getAvailableGPIOs")

namespace HAL_JSON {
    namespace GPIO_manager
    {
        namespace REST {
            void sendList();
            void setup(WEBSERVER_TYPE &srv);
        }
    }
}