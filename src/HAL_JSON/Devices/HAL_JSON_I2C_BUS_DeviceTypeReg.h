#pragma once

#include "../HAL_JSON_Device.h"
#include <Wire.h>

namespace HAL_JSON {

    typedef Device* (*I2C_HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type, TwoWire& wire);
    typedef bool (*I2C_HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    typedef struct I2C_DeviceTypeDef {
        const char* typeName;
        I2C_HAL_DEVICE_CREATE_FUNC Create_Function;
        I2C_HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } I2C_DeviceTypeDef ;

    extern const I2C_DeviceTypeDef I2C_DeviceRegistry[];
    const I2C_DeviceTypeDef* GetI2C_DeviceTypeDef(const char* type);

}