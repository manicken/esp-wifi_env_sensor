
#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    OneWireTempAutoRefresh::OneWireTempAutoRefresh(std::function<void()> _requestTemperatures, std::function<void()> _readAll)
        : requestTemperatures(_requestTemperatures), readAll(_readAll) {
            if (!requestTemperatures || !readAll) {
                Serial.println(F("ERROR @ OneWireTempAutoRefresh - Callback pointers cannot be null"));
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
                    else
                        Serial.println(F("WARNING @ OneWireTempAutoRefresh - requestTemperatures function-pointer is not set"));
                    state = State::WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case State::WAITING_FOR_CONVERSION:
                if (now - lastStart >= HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS) {
                    if (readAll != nullptr)
                        readAll();
                    else
                        Serial.println(F("WARNING @ OneWireTempAutoRefresh - readAll function-pointer is not set"));
                    lastUpdateMs = now;
                    state = State::IDLE;
                }
                break;
        }
	}

    double OneWireTempAutoRefresh::ParseRefreshTime(const JsonVariant &value) {
        if (value.is<float>() || value.is<double>()) {
            return value.as<double>();
        } else if (value.is<const char*>()) {
            return atof(value.as<const char*>());
        } else if (value.is<uint32_t>()) {
            return static_cast<double>(value.as<uint32_t>());
        }
        return -1.0; // Invalid
    }

    uint32_t OneWireTempAutoRefresh::ParseRefreshTimeMs(const JsonVariant &jsonObj) {
        double rawSec = 1.0;
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_REFRESHTIME_SEC)) {
            double val = ParseRefreshTime(jsonObj[HAL_JSON_KEYNAME_REFRESHTIME_SEC]);
            if (val >= 1.0) rawSec = val;
        }
        else if (jsonObj.containsKey(HAL_JSON_KEYNAME_REFRESHTIME_MIN)) {
            double val = ParseRefreshTime(jsonObj[HAL_JSON_KEYNAME_REFRESHTIME_MIN]);
            val *= 60.0;
            if (val >= 1.0) rawSec = val;
        }
        else {
            GlobalLogger.Warn(F("refreshrate is not set default is:"),String(HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS).c_str());
        }
        return static_cast<uint32_t>(round(rawSec * 1000));
    }

    String OneWireTempAutoRefresh::ToString() {
        String ret;
        ret += "\"refreshTimeMs\":" + String(refreshTimeMs + HAL_JSON_ONE_WIRE_TEMP_CONVERSION_TIME_MS);
        return ret;
    }
}