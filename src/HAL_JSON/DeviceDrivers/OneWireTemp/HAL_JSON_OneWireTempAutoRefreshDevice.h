#pragma once

#include <Arduino.h>
#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_DeviceTypeDefNames.h"
#include "../../ArduinoJSON_ext.h"

#define HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS 1000

namespace HAL_JSON {

    class OneWireTempAutoRefreshDevice : public Device {
        

    private:
        enum class State { IDLE, WAITING_FOR_CONVERSION };
        
        uint32_t lastUpdateMs = 0;

        State state = State::IDLE;
        uint32_t lastStart = 0;

        

    protected:
        uint32_t refreshTimeMs = HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS;
        // Make constructor protected to prevent external instantiation
        OneWireTempAutoRefreshDevice() = default;

        virtual void requestTemperatures() = 0;
        virtual void readAll() = 0;

    public:
        static double ParseRefreshTime(const JsonVariant &jsonObj);
        static uint32_t ParseRefreshTimeMs(const JsonVariant &value);
        void loop() override;
    };

    
}