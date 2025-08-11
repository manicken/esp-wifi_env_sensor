
#pragma once


#include <Arduino.h>

#include <ArduinoJson.h>
#include <stdlib.h>

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
        
        CachedDeviceAccess(const char* uidPath, const char* funcName = nullptr);
        Device* GetDevice();
    };

}