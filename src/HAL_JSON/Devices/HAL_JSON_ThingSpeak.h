
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"

namespace HAL_JSON {

    class ThingSpeak : public Device {
    private:

    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        ThingSpeak(const JsonVariant &jsonObj, const char* type);

        String ToString() override;
    };
}