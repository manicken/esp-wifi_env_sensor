#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"

class AsStringParameter : public OnTickExtParameters
{
public:
    AsStringParameter(JsonVariant json):OnTickExtParameters(0,1)
    {
        char buff[256];
        serializeJson(json, buff);
        jsonStr = std::string(buff);
    }
    std::string jsonStr;
};


namespace TimeAlarmsFromJson
{
    struct DayLookupTable {
        const char* abbreviation;
        timeDayOfWeek_t dayOfWeek;
    };
    // Define a lookup table for day abbreviations to corresponding enum values
    const DayLookupTable dayLookupTable[] = {
        {"mon", timeDayOfWeek_t::dowMonday},
        {"tue", timeDayOfWeek_t::dowTuesday},
        {"wed", timeDayOfWeek_t::dowWednesday},
        {"thu", timeDayOfWeek_t::dowThursday},
        {"fri", timeDayOfWeek_t::dowFriday},
        {"sat", timeDayOfWeek_t::dowSaturday},
        {"sun", timeDayOfWeek_t::dowSunday}
    };

    #define DEBUG_UART Serial1

    typedef struct NameToFunction {
        std::string name;
        OnTick_t onTick; // function pointer for simple non parameter callback
        OnTickExt_t onTickExt; // function pointer for ext parameter based callbacks
    } Name2Func;
    
    int FuncCount = 0;
    NameToFunction* nameToFuncList;

    
    typedef struct JsonBaseVars {
        std::string funcName;
        int h;
        int m;
        int s;
    } JsonBaseVars;

    

    bool LoadJson(String filePath);
    void ParseItem(JsonVariant json);
    void SetFunctionTable(NameToFunction* list, int count);
    OnTick_t GetFunction(std::string name);
    OnTickExt_t GetFunctionExt(std::string name);
    bool GetJsonBaseVars(JsonVariant &json, JsonBaseVars &vars);
    timeDayOfWeek_t GetTimerAlarmsDOW(std::string sDOW);

    bool LoadJson(String filePath)
    {
        DEBUG_UART.println("TimeAlarmsFromJson LoadJson start");
        
        int size = LittleFS_ext::getFileSize(filePath);
        if (size == -1) {DEBUG_UART.println(F("LoadJson file error getting file size -1")); return false; }
        
        DEBUG_UART.printf("file size:%d\r\n", size);

        char* buff = (char*)malloc(size+2);
        if (buff == NULL) { DEBUG_UART.println(F("LoadJson malloc fail")); return false; }

        if (LittleFS_ext::load_from_file(filePath, buff) == false) {
            DEBUG_UART.println(F("LoadJson LittleFS_ext::load_from_file error"));
            free(buff);
            return false;
        }
        DynamicJsonDocument jsonDoc(2048);
        DeserializationError jsonStatus = deserializeJson(jsonDoc, buff);
        DEBUG_UART.printf("deserialized json document size:%d\r\n", jsonDoc.memoryUsage());
        if (jsonStatus != DeserializationError::Ok) {
            free(buff);
            DEBUG_UART.printf("LoadJson DeserializationError: %d",(int)jsonStatus.code()); DEBUG_UART.println();
            return false;
        }

        DEBUG_UART.printf(" %u found schedules\r\n", jsonDoc.size());
        // clear both timers and alarms (ALL)
        for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
            Alarm.free(id);
        }
        size_t itemCount = jsonDoc.size();
        for (uint8_t i = 0; i < itemCount; i++) {
            ParseItem(jsonDoc[i]);
        }
        DEBUG_UART.println("TimeAlarmsFromJson LoadJson end");
        free(buff);
        return true;
    }

    void ParseItem(JsonVariant json)
    {
        //Alarm.alarmOnce();
        //Alarm.alarmRepeat(); 
        //Alarm.timerOnce(); // timer only
        //Alarm.timerRepeat(); // timer only
        //Alarm.triggerOnce(); // dtExplicitAlarm only

        if (json.containsKey("mode") == false) return;
        std::string mode = json["mode"];
        if (mode == "timer") // timer repeat only
        {
            JsonBaseVars vars;
            if (GetJsonBaseVars(json, vars) == false) return;
            AlarmID_t id=-1;
            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"].as<JsonVariant>());
                id = Alarm.timerRepeat(vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                id = Alarm.timerRepeat(vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }
            DEBUG_UART.printf("added timer repeat %d:%d:%d  %lld   %lld\r\n", vars.h, vars.m, vars.s, Alarm.read(id), now());
        }
        else if (mode == "daily")
        {
            JsonBaseVars vars;
            if (GetJsonBaseVars(json, vars) == false) return;
            AlarmID_t id=-1;
            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"].as<JsonVariant>());
                id = Alarm.alarmRepeat(vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                id = Alarm.alarmRepeat(vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }
            
            DEBUG_UART.printf("added alarm daily %d:%d:%d  %lld  %lld  %lld\r\n", vars.h, vars.m, vars.s, Alarm.read(id), Alarm.getNextTrigger(id), now());
        }
        else if (mode == "weekly")
        {
            JsonBaseVars vars;
            if (GetJsonBaseVars(json, vars) == false) return;

            if (json.containsKey("D") == false) return; // day of week
            timeDayOfWeek_t dow = GetTimerAlarmsDOW(json["D"]);

            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"].as<JsonVariant>());
                Alarm.alarmRepeat(dow, vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                Alarm.alarmRepeat(dow, vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }
            DEBUG_UART.println("added alarm weekly");
        }
        else if (mode == "explicit")
        {
            if (json.containsKey("func") == false) return;
            std::string funcName = json["func"];
            tmElements_t tm;
            if(json.containsKey("Y")) tm.Year = json["Y"]; else tm.Year = 0;
            if(json.containsKey("M")) tm.Month = json["M"]; else tm.Month = 0;
            if(json.containsKey("d")) tm.Day = json["d"]; else tm.Day = 0;
            if(json.containsKey("h")) tm.Hour = json["h"]; else tm.Hour = 0;
            if(json.containsKey("m")) tm.Minute = json["m"]; else tm.Minute = 0;
            if(json.containsKey("s")) tm.Second = json["s"]; else tm.Second = 0;
            time_t dateTime = makeTime(tm);
            if(json.containsKey("params")) {
                JsonVariant jsonVar = json["params"].as<JsonVariant>();
                AsStringParameter *params = new AsStringParameter(jsonVar);
                Alarm.triggerOnce(dateTime, GetFunctionExt(funcName), params);
            }
            else {
                Alarm.triggerOnce(dateTime, GetFunction(funcName));
            }
            DEBUG_UART.println("added alarm explicit");
        }
    }

    void SetFunctionTable(NameToFunction* list, int count)
    {
        FuncCount = count;
        nameToFuncList = list;
    }

    OnTick_t GetFunction(std::string name) {
        for (int i = 0; i < FuncCount; i++) {
            if (nameToFuncList[i].name == name)
                return nameToFuncList[i].onTick;
        }
        return nullptr;
    }

    OnTickExt_t GetFunctionExt(std::string name) {
        for (int i = 0; i < FuncCount; i++) {
            if (nameToFuncList[i].name == name)
                return nameToFuncList[i].onTickExt;
        }
        return nullptr;
    }

    bool GetJsonBaseVars(JsonVariant &json, JsonBaseVars &vars)
    {
        if (json.containsKey("func") == false) return false;
        vars.funcName = (std::string)json["func"];
        if(json.containsKey("h")) vars.h = json["h"]; else vars.h = 0;
        if(json.containsKey("m")) vars.m = json["m"]; else vars.m = 0;
        if(json.containsKey("s")) vars.s = json["s"]; else vars.s = 0;
        return true;
    }

    timeDayOfWeek_t GetTimerAlarmsDOW(std::string sDOW) // short for: short DOW
    {
        /*
        if (sDOW == "mon") return timeDayOfWeek_t::dowMonday;
        else if (sDOW == "tue") return timeDayOfWeek_t::dowTuesday;
        else if (sDOW == "wed") return timeDayOfWeek_t::dowWednesday;
        else if (sDOW == "thu") return timeDayOfWeek_t::dowThursday;
        else if (sDOW == "fri") return timeDayOfWeek_t::dowFriday;
        else if (sDOW == "sat") return timeDayOfWeek_t::dowSaturday;
        else if (sDOW == "sun") return timeDayOfWeek_t::dowSunday;
        else return timeDayOfWeek_t::dowInvalid;
        */
        for (const auto& entry : dayLookupTable) {
            if (sDOW == entry.abbreviation) {
                return entry.dayOfWeek;
            }
        }
        return timeDayOfWeek_t::dowInvalid;
    }

    std::string GetShortFormDowListAsJson()
    {
        int item_Count = sizeof(dayLookupTable) / sizeof(dayLookupTable[0]);
        std::string ret = "[";
        for (int i=0;i<item_Count;i++) {
            ret += "\"" + std::string(dayLookupTable[i].abbreviation) + "\"";
            if (i < (item_Count-1)) ret += ",";
        }
        ret += "]";
        return ret;
    }
}