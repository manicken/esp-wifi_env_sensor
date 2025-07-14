#pragma once

#include <Arduino.h>
#include <ArduinoJSON.h>
#include "../Support/Logger.h"
#include "HAL_JSON_Device_GlobalDefines.h"

namespace HAL_JSON {
    bool containsKeyIgnoreCase(JsonObject obj, const char* keyToFind);

    JsonVariant getValueIgnoreCase(JsonObject obj, const char* keyToFind);

    bool ValidateJsonStringField(const JsonVariant &jsonObj, const char* keyName);

    bool ValidateJsonStringField_noContains(const JsonVariant &jsonObj, const char* keyName);

    double ParseRefreshTime(const JsonVariant &jsonObj);
    uint32_t ParseRefreshTimeMs(const JsonVariant &jsonObj, const uint32_t defaultRefreshTimeMs);

    bool ValidateUINT8(const JsonVariant& jsonObj, const char* keyName);
    bool IsUINT32(const JsonVariant& jsonObj, const char* keyName);
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* keyName, uint32_t defaultValue);
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* keyName);
    uint8_t GetAsUINT8(const JsonVariant& jsonObj, const char* keyName);
    
    inline bool IsConstChar(const JsonVariant& jsonObj) {
        return jsonObj.is<const char*>();
    }
    inline bool IsConstChar(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName].is<const char*>();
    }
    const char* GetAsConstChar(const JsonVariant& jsonObj, const char* keyName);

}