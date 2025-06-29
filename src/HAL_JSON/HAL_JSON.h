#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_DeviceTypeDefNames.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#include "../Support/Logger.h"
#include "ArduinoJSON_ext.h"


namespace HAL_JSON {
    class Manager {
    private:
        static Device** devices;
        static uint32_t deviceCount;

        static Device* CreateDeviceFromJSON(const JsonVariant &json);
        static bool VerifyDeviceJson(const JsonVariant &jsonObj);
        static Device* findDevice(UIDPath& path);
    public:
        // JSON I/O
        static bool ParseJSON(const JsonArray &jsonArray);
        static bool ReadJSON(const char* path);

        // Device operations
        static bool read(const HALReadRequest &req);
        static bool write(const HALWriteRequest &req);
        static bool read(const HALReadStringRequest &req);
        static bool write(const HALWriteStringRequest &req);

        // Maintenance
        static void loop();

        // Debug / Testing
        static void TEST();
    };
}