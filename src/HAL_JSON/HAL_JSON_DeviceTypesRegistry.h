#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Device.h"
// all HAL devices here
#include "HAL_JSON_DeviceTypeDefNames.h"
#include "DeviceDrivers/HAL_JSON_CoreDevices.h"
#include "DeviceDrivers/OneWireTemp/HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    typedef Device* (*HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json);
    typedef bool (*HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    enum class UseRootUID {
        Mandatory,
        Optional,
        Void
    };

    typedef struct DeviceTypeDef {
        UseRootUID useRootUID;
        const char* type;
        HAL_DEVICE_CREATE_FUNC Create_Function;
        HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } DeviceTypeDef ;

    extern const DeviceTypeDef DeviceRegistry[];
}