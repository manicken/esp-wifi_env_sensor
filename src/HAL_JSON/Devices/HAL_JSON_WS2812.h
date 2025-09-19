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

#include <WS2812FX.h>

namespace HAL_JSON {

    class WS2812 : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
        
    public:
        WS2812FX* ws2812fx;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        WS2812(const JsonVariant &jsonObj, const char* type);

        HALOperationResult write(const HALWriteValueByCmd& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;
        Device::ReadToHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) override;

        static HALOperationResult writeBrightness(Device* context, HALValue& val);
        static HALOperationResult writeColor(Device* context, HALValue& val);

        void loop() override;

        String ToString() override;
    };
}