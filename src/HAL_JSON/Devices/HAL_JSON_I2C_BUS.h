
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

        Device* findDevice(UIDPath& path) override;
        void loop() override;

        HALOperationResult read(const HALReadStringRequestValue& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;

        String ToString() override;
    };

    
}