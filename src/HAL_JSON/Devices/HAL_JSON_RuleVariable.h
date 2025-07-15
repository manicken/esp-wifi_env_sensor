
#pragma once

#include <Arduino.h>
#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {

    class RuleVariable : public Device {
    private:
        HALValue value;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        RuleVariable(const JsonVariant &jsonObj, const char* type);
        bool read(HALValue& val) override;
        bool write(const HALValue& val) override;

        String ToString() override;
    };
}