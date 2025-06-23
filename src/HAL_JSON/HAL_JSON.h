
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_DeviceTypeDefNames.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#include "../Support/Logger.h"



namespace HAL_JSON {
    Device* CreateDeviceFromJSON(const JsonVariant &json);
    bool VerifyDeviceJson(const JsonVariant &jsonObj);
    void ParseJSON(const JsonArray &jsonArray);

    Device* findDevice(uint64_t uid);
    bool read(const HALReadRequest &req);
    bool write(const HALWriteRequest &req);
    bool read(const HALReadStringRequest &req);
    bool write(const HALWriteStringRequest &req);
    void loop();
}