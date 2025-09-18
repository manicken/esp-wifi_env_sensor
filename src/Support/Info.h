
#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include "../Support/LittleFS_ext.h"
#include "../Support/ConvertHelper.h"
#include "Time_ext.h"
#include <ESPAsyncWebServer.h>

#if defined(ESP8266)
//#include <ESP8266WebServer.h>
#define WEBSERVER_TYPE AsyncWebServer
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#define WIFI_getChipId() ESP.getChipId()
#define WIFI_CHIPID_PREFIX "ESP_"
#elif defined(ESP32)
//#include "Support/fs_WebServer.h"
#define WEBSERVER_TYPE AsyncWebServer
#define AUTOFORMAT_ON_FAIL true
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#define WIFI_getChipId() ESP.getEfuseMac()
#define WIFI_CHIPID_PREFIX "ESP32_"
#endif

#define INFO_URL                       "/info"
#define INFO_URL_ESP_FREE_HEAP         "/esp/free_heap"
#define INFO_URL_ESP_LAST_RESET_REASON "/esp/last_reset_reason"
#ifdef ESP32
#include <esp_heap_caps.h>
#endif
namespace Info
{

    
#ifdef ESP32
    float getHeapFragmentation();
#endif
    //WEBSERVER_TYPE *webserver = nullptr;

    extern time_t startTime;

    void printESP_info(void);
    void srv_handle_info(AsyncWebServerRequest *req);
    
    bool resetReason_is_crash();
    const char* getResetReasonStr();

    std::string GetHeapInfo();

    void PrintHeapInfo();

    void setup(WEBSERVER_TYPE &srv);

    /*
// called from setup() function
void printESP_info(void);
    */

bool resetReason_is_crash(bool includeWatchdogs);


const char* getResetReasonStr();

uint64_t reverseBytes(uint64_t value);

void srv_handle_info(AsyncWebServerRequest* req);
}