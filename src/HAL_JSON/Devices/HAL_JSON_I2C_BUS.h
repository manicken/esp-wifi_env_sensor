
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

#include <Wire.h>




namespace HAL_JSON {


    class I2C_BUS : public Device {
    private:
        uint8_t sckpin = 0;
        uint8_t sdapin = 0;
        uint32_t freq = 0;

        Device** devices;
        int deviceCount;

        TwoWire* wire;

    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        I2C_BUS(const JsonVariant &jsonObj, const char* type);
        ~I2C_BUS();

        String ToString() override;
    };

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