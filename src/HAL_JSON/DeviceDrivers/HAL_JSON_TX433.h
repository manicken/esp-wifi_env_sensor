
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"

namespace HAL_JSON {

    

    class TX433 : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        TX433(const JsonVariant &jsonObj, const char* type);
        Device* findDevice(UIDPath& path);
        bool write(const HALValue &val);
        bool write(const HALWriteStringRequestValue &val);

        String ToString() override;
    };
}