#pragma once

#define HAL_JSON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_Device_GlobalDefines.h"
#include "HAL_JSON_DeviceTypesRegistry.h"

#include "../Support/Logger.h"
#include "HAL_JSON_ArduinoJSON_ext.h"

#if defined(ESP8266)
#define WEBSERVER_H
#endif
#include <ESPAsyncWebServer.h>

#define HAL_JSON_REST_API_PORT 82
#define HAL_JSON_ROOT_URL                    "/hal"
#define HAL_JSON_FILES_PATH                  F(HAL_JSON_ROOT_URL)
#define HAL_JSON_CONFIG_JSON_FILE            F(HAL_JSON_ROOT_URL "/cfg.json")
#define HAL_JSON_URL_RELOAD_JSON             (HAL_JSON_ROOT_URL "/reloadJson")
#define HAL_JSON_URL_LIST_ALL_1WIRE_DEVICES  (HAL_JSON_ROOT_URL "/listAll1wireDevices")
#define HAL_JSON_URL_GET_VALUE               (HAL_JSON_ROOT_URL "/getValue")
#define HAL_JSON_URL_LIST_ALL_1WIRE_TEMPS    (HAL_JSON_ROOT_URL "/getAll1wireTemps")
#define HAL_JSON_URL_PRINT_DEVICES           (HAL_JSON_ROOT_URL "/printDevices")

#define HAL_JSON_REST_API_WRITE_CMD          "write"
#define HAL_JSON_REST_API_READ_CMD           "read"
#define HAL_JSON_REST_API_WRITE_URL          "/" HAL_JSON_REST_API_WRITE_CMD "/"
#define HAL_JSON_REST_API_READ_URL           "/" HAL_JSON_REST_API_READ_CMD "/"

#define HAL_JSON_REST_API_UINT32_TYPE        "uint32"
#define HAL_JSON_REST_API_BOOL_TYPE          "bool"
#define HAL_JSON_REST_API_FLOAT_TYPE         "float"
#define HAL_JSON_REST_API_JSON_STR_TYPE      "json"
#define HAL_JSON_REST_API_STRING_TYPE        "string"

namespace HAL_JSON {
    class Manager {
    private:
        static Device** devices;
        static int deviceCount;
        /** calls the begin function on all loaded hal devices */
        static void begin();

        static Device* CreateDeviceFromJSON(const JsonVariant &json);
        static bool VerifyDeviceJson(const JsonVariant &jsonObj);
        static Device* findDevice(UIDPath& path);
        static void restAPI_handleWriteOrRead(AsyncWebServerRequest *request);
        static void restAPI_printAllLoadedDevices(AsyncWebServerRequest *request);
    public:
        static void setup();
        static void reloadJSON();
        // JSON I/O
        static bool ParseJSON(const JsonArray &jsonArray);
        static bool ReadJSON(const char* path);

        // Device operations
        static bool read(const HALReadRequest &req);
        static bool write(const HALWriteRequest &req);
        static bool read(const HALReadStringRequest &req);
        static bool write(const HALWriteStringRequest &req);
        static bool read(const HALReadValueByCmdReq &req);
        static bool write(const HALWriteValueByCmdReq &req);

        // Maintenance
        static void loop();
        

        // Debug / Testing
        static void TEST();
    };
}