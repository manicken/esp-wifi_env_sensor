#pragma once

#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"
#include "Time_ext.h"
#include "NTP.h"

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#define DEBUG_UART Serial1
#define WEBSERVER_TYPE ESP8266WebServer
#elif defined(ESP32)
#include <fs_WebServer.h>
#define DEBUG_UART Serial
#define WEBSERVER_TYPE fs_WebServer
#endif

class AsStringParameter : public OnTickExtParameters
{
public:
    AsStringParameter(JsonVariant json);
    std::string jsonStr;
};

namespace Scheduler
{
    #define SCHEDULER_DIR_NAME                     F("/schedule")
    #define SCHEDULER_CFG_FILE_PATH                F("/schedule/list.json")
    #define SCHEDULER_URL_GET_TIME                 F("/schedule/getTime")
    #define SCHEDULER_URL_GET_SHORT_DOWS           F("/schedule/getShortDows")
    #define SCHEDULER_URL_GET_FUNCTION_NAMES       F("/schedule/getFunctionNames")
    #define SCHEDULER_URL_GET_MAX_NUMBER_OF_ALARMS F("/schedule/getMaxNumberOfAlarms")
    #define SCHEDULER_URL_REFRESH                  F("/schedule/refresh")

    struct DayLookupTable {
        const char* abbreviation;
        timeDayOfWeek_t dayOfWeek;
    };
    

    typedef struct NameToFunction {
        std::string name;
        OnTick_t onTick; // function pointer for simple non parameter callback
        OnTickExt_t onTickExt; // function pointer for ext parameter based callbacks
    } Name2Func;

    typedef struct JsonBaseVars {
        std::string funcName;
        int h;
        int m;
        int s;
    } JsonBaseVars;

    extern TimeAlarmsClass *Scheduler;

    extern const DayLookupTable dayLookupTable[];

    bool LoadJson(String filePath);
    void ParseItem(JsonVariant json);

    OnTick_t GetFunction(std::string name);
    OnTickExt_t GetFunctionExt(std::string name);

    bool GetJsonBaseVars(JsonVariant &json, JsonBaseVars &vars);
    timeDayOfWeek_t GetTimerAlarmsDOW(std::string sDOW);
    void HandleAlarms();
    std::string GetShortFormDowListAsJson();

    void setup(WEBSERVER_TYPE &srv, NameToFunction* funcDefList, int funcDefListCount);
}