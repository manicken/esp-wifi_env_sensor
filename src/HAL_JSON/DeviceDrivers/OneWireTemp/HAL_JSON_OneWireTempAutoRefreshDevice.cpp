
#include "HAL_JSON_OneWireTempAutoRefreshDevice.h"

namespace HAL_JSON {

	void OneWireTempAutoRefreshDevice::loop() {
		uint32_t now = millis();
        switch (state) {
            case State::IDLE:
                if (now - lastUpdateMs >= refreshTimeMs) {
                    requestTemperatures();
                    state = State::WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case State::WAITING_FOR_CONVERSION:
                if (now - lastStart >= refreshTimeMs) {
                    readAll();
                    lastUpdateMs = now;
                    state = State::IDLE;
                }
                break;
        }
	}

    double OneWireTempAutoRefreshDevice::ParseRefreshTime(const JsonVariant &value) {
        if (value.is<float>() || value.is<double>()) {
            return value.as<double>();
        } else if (value.is<const char*>()) {
            return atof(value.as<const char*>());
        } else if (value.is<uint32_t>()) {
            return static_cast<double>(value.as<uint32_t>());
        }
        return -1.0; // Invalid
    }

    uint32_t OneWireTempAutoRefreshDevice::ParseRefreshTimeMs(const JsonVariant &jsonObj) {
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
}