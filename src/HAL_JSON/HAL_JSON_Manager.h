#pragma once

#define HAL_JSON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_Device_GlobalDefines.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#include "../Support/Logger.h"
#include "HAL_JSON_ArduinoJSON_ext.h"



namespace HAL_JSON {
    class Manager {
    private:
        static Device** devices;
        static int deviceCount;
        
        static Device* CreateDeviceFromJSON(const JsonVariant &json);
        static bool VerifyDeviceJson(const JsonVariant &jsonObj);
        
        static int reloadVersion;

    public:
        // getters
        static int DeviceCount();
        static int* ReloadVersionPtr();
        // init
        /** calls the begin function on all loaded hal devices */
        static void begin();
        static void setup();
        // JSON I/O
        static bool ParseJSON(const JsonArray &jsonArray);
        static bool ReadJSON(const char* path);

        // Device operations
        static Device* findDevice(UIDPath& path);
        static bool read(const HALReadRequest &req);
        static bool write(const HALWriteRequest &req);
        static bool read(const HALReadStringRequest &req);
        static bool write(const HALWriteStringRequest &req);
        static bool read(const HALReadValueByCmdReq &req);
        static bool write(const HALWriteValueByCmdReq &req);

        // Maintenance
        static void loop();
        
        static std::string ToString();

        // Debug / Testing
        static void TEST();
    };
}