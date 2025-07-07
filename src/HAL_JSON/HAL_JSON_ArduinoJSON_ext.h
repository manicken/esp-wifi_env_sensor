#pragma once

#include <Arduino.h>
#include <ArduinoJSON.h>
#include "../Support/Logger.h"
#include "HAL_JSON_Device_GlobalDefines.h"

namespace HAL_JSON {
    bool containsKeyIgnoreCase(JsonObject obj, const char* keyToFind);

    JsonVariant getValueIgnoreCase(JsonObject obj, const char* keyToFind);

    bool ValidateJsonStringField(const JsonVariant &jsonObj, const char* key);

    bool ValidateJsonStringField_noContains(const JsonVariant &jsonObj, const char* key);

    double ParseRefreshTime(const JsonVariant &jsonObj);
    uint32_t ParseRefreshTimeMs(const JsonVariant &jsonObj, const uint32_t defaultRefreshTimeMs);

    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* name, uint32_t defaultValue);
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* name);
    bool IsUINT32(const JsonVariant& jsonObj, const char* name);
    inline bool IsConstChar(const JsonVariant& jsonObj) {
        return jsonObj.is<const char*>();
    }
    inline bool IsConstChar(const JsonVariant& jsonObj, const char* name) {
        return jsonObj[name].is<const char*>();
    }
    const char* GetAsConstChar(const JsonVariant& jsonObj, const char* name);

}