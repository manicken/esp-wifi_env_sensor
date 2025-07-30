
#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    OneWireTempAutoRefresh::OneWireTempAutoRefresh(std::function<void()> _requestTemperatures, std::function<void()> _readAll, uint32_t _refreshTimeMs)
        : requestTemperatures(_requestTemperatures), readAll(_readAll) {
            SetRefreshTimeMs(_refreshTimeMs); // to include "convertion"
            if (!requestTemperatures || !readAll) {
                Serial.println(F("ERR@OWTAR")); // short error that can be searched for in code
                //abort();  // or throw if exceptions are enabled
            }
    }

    void OneWireTempAutoRefresh::SetRefreshTimeMs(uint32_t _refreshTimeMs) {
        if (_refreshTimeMs > HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS)
            refreshTimeMs = _refreshTimeMs - HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS;
        else
            refreshTimeMs = 0; // or some minimum value to avoid underflow
    }

	void OneWireTempAutoRefresh::loop() {
		uint32_t now = millis();
        switch (state) {
            case State::IDLE:
                if (now - lastUpdateMs >= refreshTimeMs) {
                    if (requestTemperatures != nullptr)
                        requestTemperatures();
                    //else
                    //    Serial.println(F("W@OWTAR - rtFP"));
                    state = State::WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case State::WAITING_FOR_CONVERSION:
                if (now - lastStart >= HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS) {
                    if (readAll != nullptr)
                        readAll();
                    //else
                    //    Serial.println(F("W@OWTAR - raFP"));
                    lastUpdateMs = now;
                    state = State::IDLE;
                }
                break;
        }
	}

    

    String OneWireTempAutoRefresh::ToString() {
        String ret;
        ret += DeviceConstStrings::refreshTimeMs;
        ret += std::to_string(refreshTimeMs + HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS).c_str();
        return ret;
    }
}