#pragma once
#include <Arduino.h>
#include <ArduinoJSON.h>

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