
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
        
        CachedDeviceAccess(ZeroCopyString& uidPath, ZeroCopyString& funcName);
        Device* GetDevice();

        HALOperationResult WriteSimple(const HALValue& val);
        HALOperationResult ReadSimple(HALValue& val);
    };
    
}