

#include "HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {
    bool containsKeyIgnoreCase(JsonObject obj, const char* keyToFind) {
        for (JsonPair kv : obj) {
            if (strcasecmp(kv.key().c_str(), keyToFind) == 0) {
                return true;
            }
        }
        return false;
    }

    JsonVariant getValueIgnoreCase(JsonObject obj, const char* keyToFind) {
        for (JsonPair kv : obj) {
            if (strcasecmp(kv.key().c_str(), keyToFind) == 0) {
                return kv.value();
            }
        }
        return JsonVariant(); // null
    }

    bool ValidateJsonStringField(const JsonVariant &jsonObj, const char* key) {
        if (!jsonObj.containsKey(key)) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_STRING_VALUE_KEY, key);
            return false;
        }
        if (!jsonObj[key].is<const char*>()) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE_NOT_STRING, key);
            return false;
        }
        const char* val = jsonObj[key].as<const char*>();
        if (val == nullptr || strlen(val) == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY, key);
            return false;
        }
        return true;
    }

    bool ValidateJsonStringField_noLog(const JsonVariant &jsonObj, const char* key) {
        if (!jsonObj.containsKey(key)) {
            //GlobalLogger.Error(HAL_JSON_ERR_MISSING_STRING_VALUE_KEY, key);
            return false;
        }
        if (!jsonObj[key].is<const char*>()) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE_NOT_STRING, key);
            return false;
        }
        const char* val = jsonObj[key].as<const char*>();
        if (val == nullptr || strlen(val) == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY, key);
            return false;
        }
        return true;
    }

    double ParseRefreshTime(const JsonVariant &value) {
        if (value.is<float>() || value.is<double>()) {
            return value.as<double>();
        } else if (value.is<const char*>()) {
            return atof(value.as<const char*>());
        } else if (value.is<uint32_t>()) {
            return static_cast<double>(value.as<uint32_t>());
        }
        return -1.0; // Invalid
    }

    uint32_t ParseRefreshTimeMs(const JsonVariant &jsonObj, const uint32_t defaultRefreshTimeMs) {
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
            GlobalLogger.Warn(F("refreshrate is not set default is:"),String(defaultRefreshTimeMs).c_str());
            return defaultRefreshTimeMs;
        }
        return static_cast<uint32_t>(round(rawSec * 1000));;
    }
    bool IsUINT32(const JsonVariant& jsonObj, const char* name) {
        return jsonObj[name].is<uint32_t>();
    }

    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* name, uint32_t defaultValue) {
        return jsonObj[name] | defaultValue;
    }
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* name) {
        return jsonObj[name];
    }
    const char* GetAsConstChar(const JsonVariant& jsonObj, const char* name) {
        return jsonObj[name];
    }
}