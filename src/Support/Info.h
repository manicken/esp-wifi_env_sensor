
#include <Arduino.h>
#include <LittleFS.h>
#include "../Support/LittleFS_ext.h"
#include "Time_ext.h"

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#define WEBSERVER_TYPE ESP8266WebServer
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#define WIFI_getChipId() ESP.getChipId()
#define WIFI_CHIPID_PREFIX "ESP_"
#elif defined(ESP32)
#include "Support/fs_WebServer.h"
#define WEBSERVER_TYPE fs_WebServer
#define AUTOFORMAT_ON_FAIL true
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#define WIFI_getChipId() ESP.getEfuseMac()
#define WIFI_CHIPID_PREFIX "ESP32_"
#endif

#define INFO_URL                       F("/info")
#define INFO_URL_ESP_FREE_HEAP         F("/esp/free_heap")
#define INFO_URL_ESP_LAST_RESET_REASON F("/esp/last_reset_reason")

namespace Info
{
    WEBSERVER_TYPE *webserver = nullptr;

    time_t startTime = 0;

    void printESP_info(void);
    void srv_handle_info(void);
    
    bool resetReason_is_crash();
    const char* getResetReasonStr();

    void setup(WEBSERVER_TYPE &srv) {
        webserver = &srv;

        webserver->on(INFO_URL, srv_handle_info);
        webserver->on(INFO_URL_ESP_LAST_RESET_REASON, []() {
            std::string resetInfo = "Last Reset at: " + Time_ext::GetTimeAsString(startTime);
            resetInfo += "\nReason: " + std::string(getResetReasonStr());
            
            webserver->send(200, F("text/plain"), resetInfo.c_str());
        });
        webserver->on(INFO_URL_ESP_FREE_HEAP, []() {
            std::string ret = "Free Heap:" + std::to_string(ESP.getFreeHeap());
#if defined(ESP8266)
            ret += ", Fragmentation:" + std::to_string(ESP.getHeapFragmentation());
#endif
            webserver->send(200,F("text/plain"), ret.c_str());
        });
    }

    /*
// called from setup() function
void printESP_info(void) { 
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    DEBUG_UART.print(F("Flash real id:   ")); DEBUG_UART.printf("%08X\r\n", ESP.getFlashChipId());
    DEBUG_UART.print(F("Flash real size: ")); DEBUG_UART.printf("%u 0\r\n\r\n", realSize);

    DEBUG_UART.print(F("Flash ide  size: ")); DEBUG_UART.printf("%u\r\n", ideSize);
    DEBUG_UART.print(F("Flash ide speed: ")); DEBUG_UART.printf("%u\r\n", ESP.getFlashChipSpeed());
    DEBUG_UART.print(F("Flash ide mode:  ")); DEBUG_UART.printf("%s\r\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if(ideSize != realSize)
    {
        DEBUG_UART.println(F("Flash Chip configuration wrong!\r\n"));
    }
    else
    {
        DEBUG_UART.println(F("Flash Chip configuration ok.\r\n"));
    }
    DEBUG_UART.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
    DEBUG_UART.println();
    DEBUG_UART.println();
}*/

bool resetReason_is_crash(bool includeWatchdogs)
{
    #if defined(ESP8266)
    rst_info *info = system_get_rst_info();
    uint32 reason = info->reason;
    if (reason == rst_reason::REASON_EXCEPTION_RST)
        return true;
    if (includeWatchdogs) {
        switch (reason) {
            case rst_reason::REASON_WDT_RST: return true;
            case rst_reason::REASON_SOFT_WDT_RST: return true;
            default: return false;
        }
    }
#elif defined(ESP32)
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_PANIC) return true;
    if (includeWatchdogs) {
        switch (reason) {
            case ESP_RST_INT_WDT: return true;
            case ESP_RST_TASK_WDT: return true;
            case ESP_RST_WDT: return true;
            default: return false;
        }
    }
#endif
    return false;
}

const char* getResetReasonStr()
{
#if defined(ESP8266)
    rst_info *info = system_get_rst_info();
    uint32 reason = info->reason;
    if (reason == rst_reason::REASON_DEFAULT_RST)
        return "normal startup by power on";
    else if (reason == rst_reason::REASON_WDT_RST)
        return "hardware watch dog reset";
    else if (reason == rst_reason::REASON_EXCEPTION_RST)
        return "exception reset";
    else if (reason == rst_reason::REASON_SOFT_WDT_RST)
        return "software watch dog reset";
    else if (reason == rst_reason::REASON_SOFT_RESTART)
        return "software restart/system_restart";
    else if (reason == rst_reason::REASON_DEEP_SLEEP_AWAKE)
        return "wake up from deep-sleep";
    else if (reason == rst_reason::REASON_EXT_SYS_RST)
        return "external system reset";
    else
        return "undefined reset cause";
#elif defined(ESP32)
    esp_reset_reason_t reset_reason = esp_reset_reason();
    switch (reset_reason) {
        case ESP_RST_POWERON: return "Power-on reset";
        case ESP_RST_EXT: return "External reset";
        case ESP_RST_SW: return "Software reset";
        case ESP_RST_PANIC: return "Software reset due to panic";
        case ESP_RST_INT_WDT: return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT: return "Task watchdog reset";
        case ESP_RST_WDT: return "Other watchdog reset";
        case ESP_RST_DEEPSLEEP: return "Reset after deep sleep";
        case ESP_RST_BROWNOUT: return "Brownout reset";
        case ESP_RST_SDIO: return "SDIO reset";
        default: return "Unknown reset reason";
    }
#endif
        
}

uint64_t reverseBytes(uint64_t value) {
    return ((value & 0x00000000000000FF) << 56) |
           ((value & 0x000000000000FF00) << 40) |
           ((value & 0x0000000000FF0000) << 24) |
           ((value & 0x00000000FF000000) << 8)  |
           ((value & 0x000000FF00000000) >> 8)  |
           ((value & 0x0000FF0000000000) >> 24) |
           ((value & 0x00FF000000000000) >> 40) |
           ((value & 0xFF00000000000000) >> 56);
}

void srv_handle_info()
{
    uint32_t ideSize = ESP.getFlashChipSize();
#if defined(ESP8266)
    uint32_t realSize = ESP.getFlashChipRealSize();
#else
    uint32_t realSize = ideSize;
#endif
    FlashMode_t ideMode = ESP.getFlashChipMode();
    String srv_return_msg = "";

    srv_return_msg.concat(F("<!DOCTYPE html PUBLIC\"ISO/IEC 15445:2000//DTD HTML//EN\"><html><head><title></title></head><body>"));
#if defined(ESP8266)
    srv_return_msg.concat(F("<br>Flash real id:   ")); srv_return_msg.concat(ESP.getFlashChipId());
#elif defined(ESP32)
    srv_return_msg.concat(F("<br>Flash real id:  (ESP32 don't have this function)"));
#endif
    uint64_t macAddrBigEndian = Convert::reverseMACaddress(WIFI_getChipId());
    String hostString = String(macAddrBigEndian & 0xFFFFFF,HEX);
    hostString.toUpperCase();
    srv_return_msg.concat(F("<br>Chip short id:   "));srv_return_msg.concat(WIFI_CHIPID_PREFIX); srv_return_msg.concat(hostString);

    srv_return_msg.concat(F("<br>Flash real size: ")); srv_return_msg.concat(realSize);

    srv_return_msg.concat(F("<br>Flash ide  size: ")); srv_return_msg.concat(ideSize);
    srv_return_msg.concat(F("<br>Flash ide speed: ")); srv_return_msg.concat(ESP.getFlashChipSpeed());
    srv_return_msg.concat(F("<br>Flash ide mode:  ")); srv_return_msg.concat((ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    if(ideSize != realSize)
    {
        srv_return_msg.concat(F("<br>Flash Chip configuration wrong!\r\n"));
    }
    else
    {
        srv_return_msg.concat(F("<br>Flash Chip configuration ok.\r\n"));
    }
#if defined(ESP8266)
    srv_return_msg.concat(F("<br> ESP8266 Chip id = ")); srv_return_msg.concat(ESP.getChipId());
#endif
    srv_return_msg.concat(F("<br><br>"));
    if (LITTLEFS_BEGIN_FUNC_CALL) {
        srv_return_msg.concat(F("<br>LittleFS mounted OK"));
#if defined(ESP8266)
        FSInfo fsi;
        if (LittleFS.info(fsi)) {
            srv_return_msg.concat(F("<br>LittleFS blocksize = ")); srv_return_msg.concat(fsi.blockSize); 
            srv_return_msg.concat(F("<br>LittleFS maxOpenFiles = ")); srv_return_msg.concat(fsi.maxOpenFiles); 
            srv_return_msg.concat(F("<br>LittleFS maxPathLength = ")); srv_return_msg.concat(fsi.maxPathLength); 
            srv_return_msg.concat(F("<br>LittleFS pageSize = ")); srv_return_msg.concat(fsi.pageSize); 
            srv_return_msg.concat(F("<br>LittleFS totalBytes = ")); srv_return_msg.concat(fsi.totalBytes); 
            srv_return_msg.concat(F("<br>LittleFS usedBytes = ")); srv_return_msg.concat(fsi.usedBytes); 
        }
        else
#elif defined(ESP32)
        srv_return_msg.concat(F("<br>LittleFS totalBytes = ")); srv_return_msg.concat(LittleFS.totalBytes());
        srv_return_msg.concat(F("<br>LittleFS usedBytes = ")); srv_return_msg.concat(LittleFS.usedBytes()); 
#else
            srv_return_msg.concat(F("<br>LittleFS info not implemented"));
#endif

        srv_return_msg.concat("<br><br>Files:<br>");
        
        //LittleFS_ext::listDir(DEBUG_UART,"/", 0);
        LittleFS_ext::listDir(srv_return_msg, true, "/", 0);
    }
    else
        srv_return_msg.concat(F("<br>LittleFS Fail to mount"));

    srv_return_msg.concat(F("</body></html>"));
    webserver->send(200, "text/html", srv_return_msg);
    //server.sendContent(srv_return_msg);

    //server.sendContent("");
}
}