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

#include <ArduinoJson.h>
#include <stdlib.h>
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"

#include "../Support/Logger.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Device.h"
#include "HAL_JSON_Manager.h"


namespace HAL_JSON {

    class CachedDeviceAccess {
    private:
        const int* currentVersion;
        int cachedVersion;
        UIDPath path;
        Device* device;
        
    public:
        /** 
         * some devices can give direct access to the internal value
         * so in those cases this will allow that
         */
        HALValue* valueDirectAccessPtr;
        /** 
         * allows direct access to functions by name, 
         * or actually sub values defined by using # in uidPath#subItem
         */
        Device::ReadToHALValue_FuncType readToHalValueFunc;
        Device::ReadToHALValue_FuncType writeFromHalValueFunc;
        
        CachedDeviceAccess(ZeroCopyString& uidPath, ZeroCopyString& funcName);
        Device* GetDevice();

        HALOperationResult WriteSimple(const HALValue& val);
        HALOperationResult ReadSimple(HALValue& val);
    };
    
}