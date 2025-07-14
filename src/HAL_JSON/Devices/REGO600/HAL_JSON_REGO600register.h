
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../../Support/CharArrayHelpers.h"
#include "../../../Support/ConvertHelper.h"
//#include "../../../Drivers/REGO600.h"

#define HAL_JSON_KEYNAME_REGO600_OPCODE     "opcode"
#define HAL_JSON_KEYNAME_REGO600_ADDRESS    "addr"
#define HAL_JSON_KEYNAME_REGO600_VALUE_TYPE "valuetype"

namespace HAL_JSON {

    class REGO600register : public Device {
        enum class ValueType {
            FLOAT,
            INT,
            UINT,
            BOOL // will be returned as integer 0 or 1
        };
    public:
        //uint32_t opcode;  // need to be public for the moment
        //uint32_t address; // need to be public for the moment
        ValueType valueType;
        uint32_t value;   // need to be public for the moment
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        REGO600register(const JsonVariant &jsonObj, const char* type);

        bool read(HALValue& val) override;
        String ToString() override;
    };
}