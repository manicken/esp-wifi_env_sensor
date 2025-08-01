#pragma once


#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_Device_GlobalDefines.h"


namespace HAL_JSON {
    typedef Device* (*HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type);
    typedef bool (*HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    enum class UseRootUID {
        Mandatory,
        Optional,
        Void
    };

    typedef struct DeviceTypeDef {
        UseRootUID useRootUID;
        const char* typeName;
        HAL_DEVICE_CREATE_FUNC Create_Function;
        HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } DeviceTypeDef ;

    extern const DeviceTypeDef DeviceRegistry[];

}