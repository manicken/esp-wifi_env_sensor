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

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"
#include "../HAL_JSON_CachedDeviceAccess.h"

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#endif

namespace HAL_JSON {

    struct ThingSpeakField {
        int index;
        CachedDeviceAccess* cda;
    };

    class ThingSpeak : public Device {
    private:
        HTTPClient http;
        WiFiClient wifiClient;
        static const char TS_ROOT_URL[];
        char API_KEY[17];
        ThingSpeakField* fields;
        int fieldCount;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        ThingSpeak(const JsonVariant &jsonObj, const char* type);
        ~ThingSpeak();
        HALOperationResult exec() override;

        String ToString() override;
    };
}