
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../../Drivers/REGO600.h"
#include "HAL_JSON_REGO600register.h"


namespace HAL_JSON {

    class REGO600 : public Device {
    private:
        uint8_t rxPin = 0;
        uint8_t txPin = 0;
        /** this is only logical devices */
        size_t registerItemCount = 0; // used by both registerItems and requestList
        REGO600register** registerItems = nullptr;
        Drivers::REGO600::Request** requestList = nullptr;
        Drivers::REGO600* rego600 = nullptr;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        REGO600(const JsonVariant &jsonObj, const char* type);
        ~REGO600();
        void loop() override;
        void begin() override;
        String ToString() override;
    };
}