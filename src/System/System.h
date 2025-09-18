#pragma once

#include <ESPAsyncWebServer.h>

#define MAIN_URLS_FORMAT_LITTLE_FS      "/formatLittleFs"
#define MAIN_URLS_MKDIR                 "/mkdir"

#define AUTOFORMAT_ON_FAIL true

#if defined(ESP8266)
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif defined(ESP32)
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#endif

namespace System {

    void Setup();

    #if defined(ESP32)
    bool InitSD_MMC();
    void Start_MDNS();
    #endif

    void failsafeLoop();

    void initWebServerHandlers(AsyncWebServer& webserver);

}