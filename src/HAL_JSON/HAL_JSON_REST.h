#pragma once

#include <Arduino.h>
//#include <ArduinoJson.h>
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
//#include "../Support/ConvertHelper.h"

//#include "HAL_JSON_Device_GlobalDefines.h"

#include "../Support/ZeroCopyString.h"
#include "HAL_JSON_Manager.h"
#include "HAL_JSON_CommandExecutor.h"

#define HAL_JSON_REST_API_PORT 82

namespace HAL_JSON {
    class REST {
    private:
        static AsyncWebServer* asyncWebserver;
    public:
        static void setup();
    };
}