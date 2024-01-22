
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include "LittleFS_ext.h"

namespace ThingSpeak
{
    #define DEBUG_UART Serial1

    #define TS_CONFIG_JSON_FILE           "/thingspeak/cfg.json"
    #define TS_JSON_FIELD_API_KEY         "api_key"
    #define TS_JSON_FIELD_UPDATE_RATE_SEC "update_rate_sec"
    #define TS_JSON_FIELD_TEMP_FIELD      "temp_field"
    #define TS_JSON_FIELD_HUMIDITY_FIELD  "humidity_field"

    std::string api_key = "";
    int32_t update_rate_sec = 15;
    std::string temp_field = "";
    std::string humidity_field = "";

    WiFiClient wifiClient;
    HTTPClient http;
    StaticJsonDocument<256> jsonDoc;

    char jsonBuffer[128];

    bool canPost = false;

    std::string urlApi = "";

    bool loadSettings()
    {
        canPost = false;
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
        return true;
    }

    void SendData(float temp, float humidity)
    {

        urlApi = "";
        urlApi += "&"+ThingSpeak::temp_field+"=" + std::to_string(temp);
        urlApi += "&"+ThingSpeak::humidity_field+"=" + std::to_string(humidity);
        std::string url = "http://api.thingspeak.com/update?api_key="+ ThingSpeak::api_key + urlApi;
        http.begin(wifiClient, url.c_str());
        
        int httpCode = http.GET();
        if (httpCode > 0) {
            DEBUG_UART.println(F("\r\nGET request sent\r\n"));
            DEBUG_UART.println(urlApi.c_str());
        }
        else {
            DEBUG_UART.println(F("\r\nGET request FAILURE\r\n"));
            DEBUG_UART.println(urlApi.c_str());
        }
        http.end();
    }
}