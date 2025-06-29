#pragma once

#include <Arduino.h>
#include <ArduinoJSON.h>
#include "../Support/Logger.h"
#include "HAL_JSON_DeviceTypeDefNames.h"

bool containsKeyIgnoreCase(JsonObject obj, const char* keyToFind);

JsonVariant getValueIgnoreCase(JsonObject obj, const char* keyToFind);

bool ValidateJsonStringField(const JsonVariant &jsonObj, const char* key);
