
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
//#include "../../../Drivers/REGO600.h"

namespace HAL_JSON {

    class REGO600register : public Device {

    public:
        uint32_t opcode;  // need to be public for the moment
        uint32_t address; // need to be public for the moment
        uint32_t value;   // need to be public for the moment
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        REGO600register(const JsonVariant &jsonObj, const char* type);

        String ToString() override;
    };
}