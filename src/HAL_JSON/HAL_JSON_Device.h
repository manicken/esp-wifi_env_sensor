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

#include <cstdint>
#include <cstring>
#include <string>

#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_GPIO_Manager.h"


namespace HAL_JSON {
    
    enum class UIDPathMaxLength : uint8_t {
        One,
        Many
    };

    //typedef bool (*ReadToHALValue_FuncType)(HAL_JSON::Device*, HALValue&);
    

    class Device {
    protected:
        Device() = delete;
        Device(Device&) = delete;
        const char* type;
        bool loopTaskDone = false;
    public:
        using ReadToHALValue_FuncType = HALOperationResult (*)(Device*, HALValue&);
        Device(UIDPathMaxLength uidMaxLength, const char* type);
        virtual ~Device();

        HAL_UID uid;
        const UIDPathMaxLength uidMaxLength;
        bool LoopTaskDone();
        virtual HALOperationResult read(HALValue& val);
        virtual HALOperationResult write(const HALValue& val);
        virtual HALOperationResult read(const HALReadStringRequestValue& val);
        virtual HALOperationResult write(const HALWriteStringRequestValue& val);
        virtual HALOperationResult read(const HALReadValueByCmd& val);
        virtual HALOperationResult write(const HALWriteValueByCmd& val);
        virtual ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName);
        virtual ReadToHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName);
        virtual HALValue* GetValueDirectAccessPtr();
        /** called regulary from the main loop */
        virtual void loop();
        /** called when all hal devices has been loaded */
        virtual void begin();
        /** used to find sub/leaf devices @ "group devices" */
        virtual Device* findDevice(UIDPath& path);

        /** Executes a device action that requires no parameters. */
        virtual HALOperationResult exec();
        /** Executes a device action with a provided command string. */
        virtual HALOperationResult exec(ZeroCopyString& cmd);

        virtual String ToString();

        static bool DisabledInJson(const JsonVariant& jsonObj);

        static Device* findInArray(Device** devices, int deviceCount, UIDPath& path, Device* currentDevice);
    };

    
    
#if defined(ESP32)
//#define HAL_JSON_DEVICE_CONST_STRINGS_USE_F_PREFIX
#endif

#ifdef HAL_JSON_DEVICE_CONST_STRINGS_USE_F_PREFIX
#define HAL_JSON_DEVICE_CONST_STR_DECLARE(name) extern const __FlashStringHelper* name
#define HAL_JSON_DEVICE_CONST_STR_DEFINE(name, value) const __FlashStringHelper* name = F(value)
#else
#define HAL_JSON_DEVICE_CONST_STR_DECLARE(name) extern const char* name
#define HAL_JSON_DEVICE_CONST_STR_DEFINE(name, value) const char* name = value
#endif

    namespace DeviceConstStrings {
        HAL_JSON_DEVICE_CONST_STR_DECLARE(uid);
        /** "\"type\":\"" */
        HAL_JSON_DEVICE_CONST_STR_DECLARE(type);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(pin);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(value);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(valueStartWithComma);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(refreshTimeMs);

    }

    
} // namespace HAL
