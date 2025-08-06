
#pragma once


#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {

    class RuleVariableWriteOnlyTest : public Device {
    private:
        HALValue value;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        RuleVariableWriteOnlyTest(const JsonVariant &jsonObj, const char* type);
        HALDeviceOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
}