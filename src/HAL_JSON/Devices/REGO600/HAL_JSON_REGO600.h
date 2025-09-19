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

#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_REGO600register.h"
#include "../../../Drivers/REGO600.h"

namespace HAL_JSON {

    class REGO600 : public Device {
    private:
        uint32_t refreshTimeMs = 0;
        uint8_t rxPin = 0;
        uint8_t txPin = 0;
        /** this is only logical devices */
        int registerItemCount = 0; // used by both registerItems and requestList
        REGO600register** registerItems = nullptr;
        Drivers::REGO600::Request** requestList = nullptr;
        Drivers::REGO600* rego600 = nullptr;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        REGO600(const JsonVariant &jsonObj, const char* type);
        ~REGO600();
        void loop() override;
        void begin() override;
        String ToString() override;
    };
}