
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

#include <WS2812FX.h>

namespace HAL_JSON {

    class WS2812 : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
        
    public:
        WS2812FX* ws2812fx;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        WS2812(const JsonVariant &jsonObj, const char* type);

        HALOperationResult write(const HALWriteValueByCmd& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;
        Device::ReadToHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) override;

        static HALOperationResult writeBrightness(Device* context, HALValue& val);
        static HALOperationResult writeColor(Device* context, HALValue& val);

        void loop() override;

        String ToString() override;
    };
}