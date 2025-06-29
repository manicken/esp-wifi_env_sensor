

#include "ArduinoJSON_ext.h"

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
        GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY_, key);
        return false;
    }
    if (!jsonObj[key].is<const char*>()) {
        GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE_, key);
        return false;
    }
    const char* val = jsonObj[key].as<const char*>();
    if (val == nullptr || strlen(val) == 0) {
        GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY_, key);
        return false;
    }
    return true;
}

