
#pragma once


#include <Arduino.h> // Needed for String class

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
        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;
        HALValue* GetValueDirectAccessPtr() override;

        String ToString() override;
    };
}