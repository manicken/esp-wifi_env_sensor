
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"
#include "Time_ext.h"
#include "NTP.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#define DEBUG_UART Serial1
#elif defined(ESP32)
#include <WiFi.h>
#include <fs_WebServer.h>
#include "mimetable.h"
#include <mdns.h>
#define DEBUG_UART Serial
#endif

#include "Scheduler.h"

AsStringParameter::AsStringParameter(JsonVariant json):OnTickExtParameters(0,1)
{
    char buff[256];
    serializeJson(json, buff);
    jsonStr = std::string(buff);
}

namespace Scheduler
{
#ifdef ESP8266
    ESP8266WebServer *server = nullptr;
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
#endif
    int FuncCount = 0;
    NameToFunction* nameToFuncList = nullptr;

    TimeAlarmsClass *Scheduler = nullptr;

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

    void HandleAlarms() {
        if (Scheduler != nullptr) Scheduler->delay(0);
        //else // this happens when there are no file loaded, i.e. when a schedule file is not found
        //    Serial.println("Scheduler is null");
    }

    bool LoadJson(String filePath)
    {
        DEBUG_UART.println(F("TimeAlarmsFromJson LoadJson start"));

        if (LittleFS.exists(filePath) == false) { DEBUG_UART.print(F("LoadJson file not found: ")); DEBUG_UART.println(filePath); return false; }
        
        int size = LittleFS_ext::getFileSize(filePath);
        if (size == -1) {DEBUG_UART.println(F("LoadJson file error getting file size -1")); return false; }
        
        DEBUG_UART.print(F("file size: ")); DEBUG_UART.println(size);

        //char* buff = (char*)malloc(size+2);
        char *buff = new char[size+2];
        if (buff == nullptr) { DEBUG_UART.println(F("LoadJson malloc fail")); return false; }

        if (LittleFS_ext::load_from_file(filePath, buff) == false) {
            DEBUG_UART.println(F("LoadJson LittleFS_ext::load_from_file error"));
            //free(buff);
            delete[] buff;
            return false;
        }
        DynamicJsonDocument jsonDoc(2048);
        DeserializationError jsonStatus = deserializeJson(jsonDoc, buff);
        DEBUG_UART.print(F("deserialized json document size: ")); DEBUG_UART.println(jsonDoc.memoryUsage());
        if (jsonStatus != DeserializationError::Ok) {
            //free(buff);
            delete[] buff;
            DEBUG_UART.printf("LoadJson DeserializationError: "); DEBUG_UART.println((int)jsonStatus.code());
            return false;
        }
        size_t itemCount = jsonDoc.size();
        DEBUG_UART.print(itemCount); DEBUG_UART.println(F(" found schedules"));
        
        // clear both timers and alarms (ALL)
        //for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
        //    Alarm.free(id);
        //} this was the old way before dynamic allocated alarms

        uint8_t nrOfActiveAlarms = 0;
        for (uint8_t i = 0;i<itemCount;i++) {
            if (jsonDoc[i].containsKey("disabled") == false) nrOfActiveAlarms++; 
        }
        if (Scheduler != nullptr) {
            //if (Scheduler->getCurrentNrOfAlarms() < nrOfActiveAlarms) {
                delete Scheduler; // delete old instance
               // Scheduler = new TimeAlarmsClass(nrOfActiveAlarms);
            //}
            //else
            //    Scheduler->clear()
        }
        //else // this happens only the very first time LoadJson is called

        Scheduler = new TimeAlarmsClass(nrOfActiveAlarms);

        // sync time with NTP server
        NTP::NTPConnect();
        tmElements_t now2;
        breakTime(time(nullptr), now2);
        int year = (int)now2.Year + 1970;
        setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);

        for (uint8_t i = 0; i < itemCount; i++) {
            ParseItem(jsonDoc[i]);
        }
        DEBUG_UART.println(F("TimeAlarmsFromJson LoadJson end"));
        //free(buff);
        delete[] buff;
        return true;
    }

    void ParseItem(JsonVariant json)
    {
        //Alarm.alarmOnce();
        //Alarm.alarmRepeat(); 
        //Alarm.timerOnce(); // timer only
        //Alarm.timerRepeat(); // timer only
        //Alarm.triggerOnce(); // dtExplicitAlarm only
        if (json.containsKey("disabled") == true) return;
        //if (json["enabled"] == false) return;

        if (json.containsKey("mode") == false) return;
        std::string mode = json["mode"];
        if (mode == "timer") // timer repeat only
        {
            JsonBaseVars vars;
            if (GetJsonBaseVars(json, vars) == false) return;
            AlarmID_t id=-1;
            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"].as<JsonVariant>());
                id = Scheduler->timerRepeat(vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                id = Scheduler->timerRepeat(vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }

            DEBUG_UART.printf("added timer repeat %d:%d:%d  now:%s   Scheduler->read(id):%s   Scheduler->getNextTrigger():%s   Scheduler->getNextTrigger(id):%s\r\n", vars.h, vars.m, vars.s, Time_ext::GetTimeAsString(now()).c_str(), Time_ext::GetTimeAsString(Scheduler->read(id)).c_str() , Time_ext::GetTimeAsString(Scheduler->getNextTrigger()).c_str(), Time_ext::GetTimeAsString(Scheduler->getNextTrigger(id)).c_str());
        
        }
        else if (mode == "daily")
        {
            JsonBaseVars vars;
            if (GetJsonBaseVars(json, vars) == false) return;
            AlarmID_t id=-1;
            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"].as<JsonVariant>());
                id = Scheduler->alarmRepeat(vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                id = Scheduler->alarmRepeat(vars.h,vars.m,vars.s, GetFunction(vars.funcName));
            }
            
            DEBUG_UART.printf("added timer daily %d:%d:%d  now:%s   Scheduler->read(id):%s   Scheduler->getNextTrigger():%s   Scheduler->getNextTrigger(id):%s\r\n", vars.h, vars.m, vars.s, Time_ext::GetTimeAsString(now()), Time_ext::GetTimeAsString(Scheduler->read(id)) , Time_ext::GetTimeAsString(Scheduler->getNextTrigger()), Time_ext::GetTimeAsString(Scheduler->getNextTrigger(id)));
        
        }
        else if (mode == "weekly")
        {
            JsonBaseVars vars;
            if (GetJsonBaseVars(json, vars) == false) return;

            if (json.containsKey("D") == false) return; // day of week
            timeDayOfWeek_t dow = GetTimerAlarmsDOW(json["D"]);

            if(json.containsKey("params")) {
                AsStringParameter *params = new AsStringParameter(json["params"].as<JsonVariant>());
                Scheduler->alarmRepeat(dow, vars.h,vars.m,vars.s, GetFunctionExt(vars.funcName), params);
            }
            else {
                Scheduler->alarmRepeat(dow, vars.h,vars.m,vars.s, GetFunction(vars.funcName));
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
                Scheduler->triggerOnce(dateTime, GetFunctionExt(funcName), params);
            }
            else {
                Scheduler->triggerOnce(dateTime, GetFunction(funcName));
            }
            DEBUG_UART.println("added alarm explicit");
        }
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
        vars.funcName = (std::string)json["func"].as<std::string>();
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

#ifdef ESP8266
    void setup(ESP8266WebServer &srv) {
#elif defined(ESP32)
    void setup(fs_WebServer &srv, NameToFunction* funcDefList, int funcDefListCount) {
#endif
        server = &srv;
        FuncCount = funcDefListCount;
        nameToFuncList = funcDefList;
        NTP::NTPConnect();
        tmElements_t now2;
        breakTime(time(nullptr), now2);
        int year = (int)now2.Year + 1970;
        setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
        

        DEBUG_UART.printf("nameToFunctionList_Count:%d\r\n", funcDefListCount);
        
        srv.on(SCHEDULER_URL_REFRESH, []() {
        if (LoadJson(SCHEDULER_CFG_FILE_PATH))
            server->send(200,F("text/plain"), F("schedule load json OK"));
        else
            server->send(200,F("text/plain"), F("schedule load json error"));
        });
        srv.on(SCHEDULER_URL_GET_MAX_NUMBER_OF_ALARMS, []() {
            std::string ret = std::to_string(dtNBR_ALARMS);
            server->send(200, F("text/plain"), ret.c_str());
        });
        srv.on(SCHEDULER_URL_GET_FUNCTION_NAMES, []() {

            std::string jsonStr = "{";

            for (int i=0;i<FuncCount;i++) {
                jsonStr += "\"" +  nameToFuncList[i].name + "\":\"" + ((nameToFuncList[i].onTickExt!=nullptr)?"p":"") + "\"";

                if (i < (FuncCount-1)) jsonStr += ",";
            }
            jsonStr += "}";
            server->send(200,F("text/plain"), jsonStr.c_str());
        });
        srv.on(SCHEDULER_URL_GET_SHORT_DOWS, []() {
            std::string ret = GetShortFormDowListAsJson();
            server->send(200,F("text/plain"), ret.c_str());
        });
        srv.on(SCHEDULER_URL_GET_TIME, []() {
            std::string nowstr = "{\n\"now\":\"" + Time_ext::GetTimeAsString(now()) + "\",\n";
            nowstr += "\"next trigger\":\"" + Time_ext::GetTimeAsString(Scheduler->getNextTrigger()) + "\"\n}";
            server->send(200,F("text/json"), nowstr.c_str());
        });
        LoadJson(SCHEDULER_CFG_FILE_PATH);
    }
}