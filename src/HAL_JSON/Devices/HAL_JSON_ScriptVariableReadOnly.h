
#pragma once


#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {

    class ScriptVariableReadOnly : public Device {
    private:
        HALValue value;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        ScriptVariableReadOnly(const JsonVariant &jsonObj, const char* type);
        HALOperationResult read(HALValue& val) override;

        String ToString() override;
    };
}