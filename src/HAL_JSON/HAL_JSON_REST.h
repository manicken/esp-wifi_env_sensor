#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>

#ifndef _WIN32
  #include "../Support/LittleFS_ext.h"
  #include <WiFiClient.h>
#else
  #include <LittleFS_ext.h>
#endif
#include <ESPAsyncWebServer.h> // have a stub wrapper for this

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_Manager.h"

#define HAL_JSON_REST_API_PORT 82

#define HAL_JSON_URL_RELOAD_JSON             (HAL_JSON_ROOT_URL "/reloadJson")
//#define HAL_JSON_URL_LIST_ALL_1WIRE_DEVICES  (HAL_JSON_ROOT_URL "/listAll1wireDevices")
//#define HAL_JSON_URL_GET_VALUE               (HAL_JSON_ROOT_URL "/getValue")
//#define HAL_JSON_URL_LIST_ALL_1WIRE_TEMPS    (HAL_JSON_ROOT_URL "/getAll1wireTemps")
#define HAL_JSON_URL_PRINT_DEVICES           (HAL_JSON_ROOT_URL "/printDevices")

#define HAL_JSON_REST_API_WRITE_CMD          "write"
#define HAL_JSON_REST_API_READ_CMD           "read"
#define HAL_JSON_REST_API_WRITE_URL          "/" HAL_JSON_REST_API_WRITE_CMD "/"
#define HAL_JSON_REST_API_READ_URL           "/" HAL_JSON_REST_API_READ_CMD "/"

#define HAL_JSON_REST_API_UINT32_TYPE        "uint32"
#define HAL_JSON_REST_API_BOOL_TYPE          "bool"
#define HAL_JSON_REST_API_FLOAT_TYPE         "float"
#define HAL_JSON_REST_API_JSON_STR_TYPE      "json"
#define HAL_JSON_REST_API_STRING_TYPE        "string"

namespace HAL_JSON {
    class REST {
    private:
        static AsyncWebServer* asyncWebserver;

        static void handleWriteOrRead(AsyncWebServerRequest *request);
    public:
        static void setup();
    };
}