
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

    class PCF8574x : public Device {
    private:
        uint8_t addr = 0;
        TwoWire* wire;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        PCF8574x(const JsonVariant &jsonObj, const char* type, TwoWire& wire);

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
}