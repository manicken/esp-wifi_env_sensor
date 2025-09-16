
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"
#include "../HAL_JSON_CachedDeviceAccess.h"

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#endif

namespace HAL_JSON {

    struct ThingSpeakField {
        int index;
        CachedDeviceAccess* cda;
    };

    class ThingSpeak : public Device {
    private:
        HTTPClient http;
        WiFiClient wifiClient;
        static const char TS_ROOT_URL[];
        char API_KEY[17];
        ThingSpeakField* fields;
        int fieldCount;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        ThingSpeak(const JsonVariant &jsonObj, const char* type);
        ~ThingSpeak();
        HALOperationResult exec() override;

        String ToString() override;
    };
}