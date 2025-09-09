#pragma once

#include <Arduino.h>
//#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>

#if defined(_WIN32) || defined(__linux__)
  #include <LittleFS_ext.h>
#else
  #include "../Support/LittleFS_ext.h"
  #include <WiFiClient.h>
#endif
#include <ESPAsyncWebServer.h> // have a stub wrapper for this

#include "../Support/Logger.h"
//#include "../Support/ConvertHelper.h"

//#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_ZeroCopyString.h"
#include "HAL_JSON_Manager.h"
#include "HAL_JSON_CommandExecutor.h"

#define HAL_JSON_REST_API_PORT 82

namespace HAL_JSON {
    class REST {
    private:
        static AsyncWebServer* asyncWebserver;
    public:
        static void setupRest();
    };
}