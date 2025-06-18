
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_DeviceTypeDefNames.h"
#include "HAL_JSON_DeviceTypesRegistry.h"



namespace HAL_JSON {
    Device* CreateDeviceFromJSON(JsonVariant &json);
    bool VerifyDeviceJson(JsonVariant &jsonObj);
    void ParseJSON(JsonArray &jsonArray);
}