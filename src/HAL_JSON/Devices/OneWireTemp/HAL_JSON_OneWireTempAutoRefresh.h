#pragma once


#include <Arduino.h> // Needed for String class
#include <functional>

#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"

#define HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS 1000
#define HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS 1000

namespace HAL_JSON {

    class OneWireTempAutoRefresh {
        typedef void (*Callback)();

    private:
        enum class State { IDLE, WAITING_FOR_CONVERSION };
        
        uint32_t lastUpdateMs = 0;

        State state = State::IDLE;
        uint32_t lastStart = 0;

        std::function<void()> requestTemperatures;
        std::function<void()> readAll;

    protected:
        uint32_t refreshTimeMs = HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS;


    public:
        OneWireTempAutoRefresh() = delete;
        OneWireTempAutoRefresh(OneWireTempAutoRefresh&) = delete;
        OneWireTempAutoRefresh(std::function<void()> _requestTemperatures, std::function<void()> _readAll, uint32_t _refreshTimeMs);
        void SetRefreshTimeMs(uint32_t _refreshTimeMs);
        
        void loop();

        String ToString();
    };

    
}