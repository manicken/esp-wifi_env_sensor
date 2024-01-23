
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include "LittleFS_ext.h"

namespace ThingSpeak
{
    #define DEBUG_UART Serial1
     
    const char TS_ROOT_URL[] = "http://api.thingspeak.com/update?api_key=";

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
    //char urlApi[128];
    //size_t urlApiStartIndex = 0;

    bool loadSettings()
    {
        canPost = false;
        jsonDoc.clear();
        //DEBUG_UART.printf("free6a:%ld\n",ESP.getFreeHeap());

        if(LittleFS.exists(TS_CONFIG_JSON_FILE)) {
            LittleFS_ext::load_from_file(TS_CONFIG_JSON_FILE, jsonBuffer);
            //DEBUG_UART.printf("free6b:%ld\n",ESP.getFreeHeap());
            deserializeJson(jsonDoc, jsonBuffer);
            //DEBUG_UART.printf("free6c:%ld\n",ESP.getFreeHeap());
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
        //DEBUG_UART.printf("free6y:%ld\n",ESP.getFreeHeap());
        urlApi = TS_ROOT_URL + api_key + "&" + temp_field + "=";
        //DEBUG_UART.printf("free6z:%ld\n",ESP.getFreeHeap());
        /*urlApi[0] = '\0';
        strcat(urlApi, TS_ROOT_URL);
        strcat(urlApi, api_key.c_str());
        strcat(urlApi, "&");
        strcat(urlApi, temp_field.c_str());
        strcat(urlApi, "=");
        urlApiStartIndex = strlen(urlApi);*/

        //memcpy(urlApi, TS_ROOT_URL, strlen(TS_ROOT_URL));
        //memcpy(urlApi+strlen(TS_ROOT_URL), api_key.c_str(), api_key.length() );
        
        return true;
    }

    void SendData(float temp, float humidity)
    {
        urlApi += std::to_string(temp) + "&" + humidity_field + "=" + std::to_string(humidity);
/*
        std::string tempValueString = std::to_string(temp);
        std::string humidityValueString = std::to_string(humidity);
        strcat(&urlApi[urlApiStartIndex], tempValueString.c_str());
        strcat(urlApi, "&");
        strcat(urlApi, humidity_field.c_str());
        strcat(urlApi, "=");
        strcat(urlApi, humidityValueString.c_str());*/

        //urlApi = "";
        //urlApi += "&"+ThingSpeak::temp_field+"=" + std::to_string(temp);
        //urlApi += "&"+ThingSpeak::humidity_field+"=" + std::to_string(humidity);
        //std::string url = "";//TS_ROOT_URL;
        //url..append(TS_ROOT_URL);
        // + ThingSpeak::api_key + urlApi;

        //DEBUG_UART.print(F("\r\nGET request"));
        //DEBUG_UART.println(urlApi.c_str());
        
        http.begin(wifiClient, urlApi.c_str());
        
        int httpCode = http.GET();
        if (httpCode > 0) {
            DEBUG_UART.println(F("[OK]\r\n"));
            
        }
        else {
            DEBUG_UART.println(F("[FAIL]\r\n"));
        }
        http.end();
        

       urlApi.clear();
       urlApi = TS_ROOT_URL + api_key + "&" + temp_field + "=";
    }
}