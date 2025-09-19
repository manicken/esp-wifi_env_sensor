/*

  <one line to give the program's name and a brief idea of what it does.>

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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