#pragma once

#define HAL_JSON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#if defined(_WIN32) || defined(__linux__)
#include <LittleFS_ext.h>
#else
#include "../Support/LittleFS_ext.h"
#endif
#include <string>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_Device_GlobalDefines.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#include "../Support/Logger.h"
#include "HAL_JSON_ArduinoJSON_ext.h"

#define HAL_JSON_ROOT_URL                    "/hal"
#define HAL_JSON_FILES_PATH                  F(HAL_JSON_ROOT_URL)
#define HAL_JSON_CONFIG_JSON_FILE            F(HAL_JSON_ROOT_URL "/cfg.json")

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
        static HALOperationResult read(const HALReadRequest &req);
        static HALOperationResult write(const HALWriteRequest &req);
        static HALOperationResult read(const HALReadStringRequest &req);
        static HALOperationResult write(const HALWriteStringRequest &req);
        static HALOperationResult read(const HALReadValueByCmdReq &req);
        static HALOperationResult write(const HALWriteValueByCmdReq &req);

        // Maintenance
        static void loop();
        
        static std::string ToString();

        // Debug / Testing
        static void TEST();
    };
}