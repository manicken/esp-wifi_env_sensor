
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_DeviceTypeDefNames.h"

namespace HAL_JSON {

    class Template : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        Template(const JsonVariant &jsonObj);

        String ToString() override;
    };
}