
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"
#include "../HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {

    struct ThingSpeakField {
        int index;
        CachedDeviceAccess* cda;
    };

    class ThingSpeak : public Device {
    private:
        static const char TS_ROOT_URL[];
        char API_KEY[17];
        ThingSpeakField* fields;
        int fieldCount;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        ThingSpeak(const JsonVariant &jsonObj, const char* type);

        HALOperationResult exec() override;

        String ToString() override;
    };
}