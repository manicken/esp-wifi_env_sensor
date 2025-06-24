#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include <LittleFS.h>
#include "LittleFS_ext.h"
#include "Support/ConvertHelper.h"

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#define DEBUG_UART Serial1
#define WEBSERVER_TYPE ESP8266WebServer
#define WIFI_getChipId() ESP.getChipId()
#define WIFI_CHIPID_PREFIX "ESP_"
#elif defined(ESP32)
#include <fs_WebServer.h>
#define DEBUG_UART Serial
#define WEBSERVER_TYPE fs_WebServer
#define WIFI_getChipId() (uint32_t)(ESP.getEfuseMac()>>32)
#define WIFI_CHIPID_PREFIX "ESP32_"
#endif

#define MAIN_CONFIG_FILES_PATH                  F("/")
#define MAIN_CONFIG_CONFIG_JSON_FILE            F("/cfg.json")
#define MAIN_CONFIG_URL_RELOAD_JSON             F("/MainConfig/restart")

namespace MainConfig {
    extern String mDNS_name;
    extern std::string lastJSONread_Error;
    void begin(WEBSERVER_TYPE &srv);
    bool ReadJson();
    void SetDefault_mDNS_name();
    void OnReadJsonFail();
}