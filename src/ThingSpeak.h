
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include "LittleFS_ext.h"

namespace ThingSpeak
{
    #define DEBUG_UART Serial1
     
    const char TS_ROOT_URL[] = "http://api.thingspeak.com/update?api_key=";

    #define TS_FILES_PATH                 F("/thingspeak")
    #define TS_CONFIG_JSON_FILE           F("/thingspeak/cfg.json")
    #define TS_JSON_FIELD_API_KEY         F("api_key")
    #define TS_JSON_FIELD_UPDATE_RATE_SEC F("update_rate_sec")
    #define TS_JSON_FIELD_TEMP_FIELD      F("temp_field")
    #define TS_JSON_FIELD_HUMIDITY_FIELD  F("humidity_field")

    std::string api_key = "";
    int32_t update_rate_sec = 15;
    std::string temp_field = "";
    std::string humidity_field = "";

    WiFiClient wifiClient;
    
    HTTPClient http;
    StaticJsonDocument<256> jsonDoc;

    char jsonBuffer[256];

    bool canPost = false;

    std::string urlApi;

    bool loadSettings()
    {
        canPost = false;
        jsonDoc.clear();
        if (!LittleFS.exists(TS_FILES_PATH))
            LittleFS.mkdir(TS_FILES_PATH);

        if(LittleFS.exists(TS_CONFIG_JSON_FILE)) {
            LittleFS_ext::load_from_file(TS_CONFIG_JSON_FILE, jsonBuffer);
            deserializeJson(jsonDoc, jsonBuffer);
        }
        else {
            return false;
        }

        if (jsonDoc.containsKey(TS_JSON_FIELD_API_KEY))
            api_key = (std::string)jsonDoc[TS_JSON_FIELD_API_KEY];
        else {
            return false;
        }

        if (jsonDoc.containsKey(TS_JSON_FIELD_UPDATE_RATE_SEC))
            update_rate_sec = jsonDoc[TS_JSON_FIELD_UPDATE_RATE_SEC];
        
        if (jsonDoc.containsKey(TS_JSON_FIELD_TEMP_FIELD))
            temp_field = (std::string)jsonDoc[TS_JSON_FIELD_TEMP_FIELD];
        else
            temp_field = "field1";

        if (jsonDoc.containsKey(TS_JSON_FIELD_HUMIDITY_FIELD))
            humidity_field = (std::string)jsonDoc[TS_JSON_FIELD_HUMIDITY_FIELD];
        else
            humidity_field = "field2";

        canPost = true;
        urlApi = TS_ROOT_URL + api_key + "&" + temp_field + "=";

        return true;
    }

    void SendData(float temp, float humidity)
    {
        urlApi += std::to_string(temp) + "&" + humidity_field + "=" + std::to_string(humidity);

        //DEBUG_UART.print(F("\r\nGET request"));
        //DEBUG_UART.println(urlApi.c_str());
        
        http.begin(wifiClient, urlApi.c_str());
        
        int httpCode = http.GET();
        if (httpCode > 0) DEBUG_UART.println(F("[OK]\r\n"));
        else DEBUG_UART.println(F("[FAIL]\r\n"));

        http.end();

        urlApi.clear();
        urlApi = TS_ROOT_URL + api_key + "&" + temp_field + "=";
    }
}