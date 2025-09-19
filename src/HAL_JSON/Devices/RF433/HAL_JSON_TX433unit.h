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
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "../../../Support/CharArrayHelpers.h"
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../../Drivers/RF433.h"

#define HAL_JSON_KEYNAME_TX433_MODEL "model"


namespace HAL_JSON {

    enum class TX433_MODEL {
        FixedCode,
        LearningCode
    };

    class TX433unit : public Device {
    private:
        /** this is set from root TX433 device and used when sending */
        const uint32_t pin;
        /** defines which type to send state to while using the standard write function */
        TX433_MODEL model;
        /** the part of the data that is fixed */
        uint32_t staticData;
        /** set to false when unused i.e. when the write function sets the state 
         * otherwise it will use the state that included into staticData while using the write function
        */
        bool fixedState=false;

        static bool VerifyFC_JSON(const JsonVariant &jsonObj);
        static bool VerifyLC_JSON(const JsonVariant &jsonObj);
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        TX433unit(const JsonVariant &jsonObj, const char* type, const uint32_t pin);
        TX433unit(TX433unit&) = delete;
        HALOperationResult write(const HALValue &val);

        String ToString() override;
    };
}