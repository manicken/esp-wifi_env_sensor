#include <ArduinoJson.h>
#include <LittleFS.h>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#endif
#include "LittleFS_ext.h"

#include "DeviceManager.h"

//#define TS_DEBUG_PRINT_AFTER_JSON_READ

namespace ThingSpeak
{
    int debug_dont_send_to_server = 0;
    int debug_output = 0;
    std::string lastError;

    #ifdef ESP8266
    ESP8266WebServer *server = nullptr;
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
    #define DEBUG_UART Serial
#endif
     
    const char TS_ROOT_URL[] = "http://api.thingspeak.com/update?api_key=";

    #define TS_ACTIVITY_LED_PIN            5
    #define TS_FILES_PATH                  F("/thingspeak")
    #define TS_CONFIG_JSON_FILE            F("/thingspeak/cfg.json")
    #define TS_CONFIG_JSON_FILE_URL_RELOAD F("/thingspeak/refresh")
    #define TS_URL_SEND_DATA               F("/thingspeak/sendData")
    //#define TS_JSON_FIELD_API_KEY         F("api_key")
    //#define TS_JSON_FIELD_TEMP_FIELD      F("temp_field")
    //#define TS_JSON_FIELD_HUMIDITY_FIELD  F("humidity_field")

    #define TS_CHANNEL_MAX_FIELDS 8
    struct Channel
    {
        std::string api_write_key = "";
        /**
         * here each index in the array represents a field index,
         * and each value represents that "device" uid that it will get it's value from, 
         * a value of -1 represents a unused field and will get ignored when uploading data
         */
        int uids[TS_CHANNEL_MAX_FIELDS];

        Channel() {
            for (int i=0;i<TS_CHANNEL_MAX_FIELDS;i++) uids[i] = -1;
        }
    };

    Channel* channels = nullptr;
    int channelCount = 0;

    WiFiClient wifiClient;
    
    HTTPClient http;
    
    bool canPost = false;

    bool isInteger(const char* str)
    {
        if (str == nullptr || *str == '\0') return false; // Null or empty string is not valid

        // Handle leading sign
        if (*str == '+' || *str == '-') str++;

        // Check the rest of the string
        while (*str != '\0')
        {
            if (!isdigit(*str)) return false;
            str++;
        }

        return true; // All characters are digits
    }

    bool readJson()
    {
        if (!LittleFS.exists(TS_FILES_PATH))
        {
            LittleFS.mkdir(TS_FILES_PATH);
            lastError = "dir did not exist";
            return false;
        }
        if (LittleFS.exists(TS_CONFIG_JSON_FILE) == false) {
            lastError = "cfg file did not exist";
            return false;
        }
        int size = LittleFS_ext::getFileSize(TS_CONFIG_JSON_FILE);
        char jsonBuffer[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(TS_CONFIG_JSON_FILE, jsonBuffer) == false)
        {
            lastError = "could not load json file";
            return false;
        }
        DynamicJsonDocument jsonDoc(1024);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            lastError = "deserialization failed: " + std::string(error.c_str());
            return false;
        }
        if (!jsonDoc.is<JsonObject>()) {
            lastError = "jsonDoc is not a valid JsonObject\n";
            return false;
        }
        channelCount = jsonDoc.size();
        if (channels != nullptr) { delete[] channels; channels = nullptr; }// free(channels);
        //channels = (Channel*)malloc(sizeof(Channel) * channelCount);
        channels = new Channel[channelCount];
        if (channels == nullptr){ lastError="could not allocate memory"; return false; }
        int currChIndex = 0;
        lastError = "";
        for (JsonPair ckv : jsonDoc.as<JsonObject>()) {
            if (currChIndex >= channelCount) { // this should not happen
                lastError += "currChIndex exceeds channel array size\n";
                return false;
            }
            Channel &channel = channels[currChIndex++];
            const char* api_key_str = ckv.key() ? ckv.key().c_str() : nullptr;
            if (api_key_str == nullptr) {
                lastError+="could not get api_key_str @ currChIndex:" + std::to_string(currChIndex);
                continue;
            }
            if (strncmp(api_key_str, "debug", 5) == 0) {
                if (ckv.value() == nullptr) continue; // no error print here
                if (ckv.value().is<JsonObject>() == false) continue; // no error print here
                JsonVariant debug = ckv.value();
                if (debug.containsKey("dont_send_to_server")) debug_dont_send_to_server = 1;
                else debug_dont_send_to_server = 0;
                if (debug.containsKey("debug_output")) debug_output = 1;
                else debug_output = 0;

                continue;
            }
            channel.api_write_key = std::string(api_key_str);
            if (ckv.value() == nullptr) {
                lastError+="json object value is null @ currChIndex:" + std::to_string(currChIndex) + "\n";
                continue;
            }
            if (ckv.value().is<JsonObject>() == false) {
                lastError+="json object don't contain a json object @ currChIndex:" + std::to_string(currChIndex) + "\n";
                continue;
            }
            int currIndex = -1; // debug use only
            for (JsonPair fkv : ckv.value().as<JsonObject>()) {
                currIndex++; // debug use only
                const char* fieldIndexStr = fkv.key() ? fkv.key().c_str() : nullptr;
                if (fieldIndexStr == nullptr) {
                    lastError+="fieldIndex is not a string @ currIndex:" + std::to_string(currIndex) + "\n";
                    continue;
                }
                if (isInteger(fieldIndexStr) == false) {
                    lastError+="fieldIndex is not a integer @ currIndex:" + std::to_string(currIndex) + "\n";
                    continue;
                }
                int fieldIndex = atoi(fieldIndexStr);
                if (fieldIndex < 1 || fieldIndex > (TS_CHANNEL_MAX_FIELDS)) {
                    lastError += "fieldIndex out of bounds: " + std::to_string(fieldIndex) + "\n";
                    continue;
                }
                uint32_t uid = 0;
                if (fkv.value().is<int>()) {
                    uid = fkv.value().as<int>();
                }
                else if (fkv.value().is<const char*>()) {
                    const char* uid_asciiHexString = fkv.value().as<const char*>();
                    if (uid_asciiHexString == nullptr) { lastError+="uid could not convert to a string @ fieldIndex key:" + std::to_string(fieldIndex) + "\n"; return false; }
                    char* endptr = nullptr;
                    uid = static_cast<uint32_t>(std::strtoul(uid_asciiHexString, &endptr, 16));
                    if (endptr == uid_asciiHexString) {
                        lastError += "Error @ thingspead readjson: No valid conversion could be performed (invalid input).\n";
                    } else if (*endptr != '\0') {
                        lastError += "Warning @ thingspead readjson: Additional characters after number: ";
                        lastError += std::string(endptr);
                        lastError += "\n";
                    }
                }
                else {
                    lastError+="uid is either a number of a ascii hex string value @ fieldIndex key:" + std::to_string(fieldIndex) + "\n";
                    continue;
                }
                
                channel.uids[fieldIndex-1] = uid;
            }
        }

#ifdef TS_DEBUG_PRINT_AFTER_JSON_READ
        for (int ci=0;ci<channelCount;ci++)
        {
            DEBUG_UART.println("channel api_key:" + String(channels[ci].api_write_key.c_str()));
            DEBUG_UART.println("channel fields:");
            for (int fi=0;fi<TS_CHANNEL_MAX_FIELDS;fi++)
            {
                DEBUG_UART.println("field:" + String(fi+1) + ", uid:" + String(channels[ci].uids[fi]));
            }
            
        }
#endif

        canPost = true;
        return true;
    }
    std::string floatToString(float value) {
        char buffer[32]; // Adjust size based on needs
        snprintf(buffer, sizeof(buffer), "%.6f", value); // Format as float with 6 decimals

        std::string result(buffer);

        // Remove trailing zeros
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        if (result.back() == '.') {
            result.pop_back(); // Remove decimal point if no fractional part
        }

        return result;
    }

    bool sendDataBackToWebbrowser = false;

    void SendData()
    {
        digitalWrite(TS_ACTIVITY_LED_PIN, HIGH); // TODO move this and make it more customaziable
        //DEBUG_UART.println("TS SendData");
        if (channels == nullptr) return;
        if (DeviceManager::getAllOneWireTemperatures() == false) return;

        for (int ci=0;ci<channelCount;ci++)
        {
            if (channels[ci].api_write_key.size() == 0) continue;

            std::string fieldData = "";
            for (int fi=0;fi<8;fi++)
            {
                if (channels[ci].uids[fi] == -1) continue;
                float value = 0;
                if (DeviceManager::getValue(channels[ci].uids[fi], &value) == false) continue;
                fieldData += "&field" + std::to_string(fi+1) + "=" + floatToString(value);
            }
            if (fieldData.length() == 0) continue;
            std::string urlApi = TS_ROOT_URL + channels[ci].api_write_key + fieldData;

            if (debug_dont_send_to_server == 0) {
                http.begin(wifiClient, urlApi.c_str());
                
                int httpCode = http.GET();
                if (httpCode > 0) DEBUG_UART.println(F("[OK]\r\n"));
                else DEBUG_UART.println(F("[FAIL]\r\n"));

                http.end();
            }
            if (debug_output == 1) {
                DEBUG_UART.println(urlApi.c_str());
            }
            if (sendDataBackToWebbrowser) {
                sendDataBackToWebbrowser = false;
                server->send(200, "text/html", urlApi.c_str()); // in case the request is from a url to test
            }
        }
        digitalWrite(TS_ACTIVITY_LED_PIN, LOW); // TODO move this and make it more customaziable

    }

    void reloadJson()
    {
        if (readJson())
            server->send(200,F("text/plain"), F("Thingspeak loadSettings OK"));
        else
            server->send(200,F("text/plain"), F("Thingspeak loadSettings error"));
    }

    void htmlSendDataTest()
    {
        sendDataBackToWebbrowser = true;
        SendData();
    }

   #ifdef ESP8266
    void setup(ESP8266WebServer &srv) {
#elif defined(ESP32)
    void setup(fs_WebServer &srv) {
#endif
        server = &srv;
        srv.on(TS_CONFIG_JSON_FILE_URL_RELOAD, reloadJson);
        srv.on(TS_URL_SEND_DATA, htmlSendDataTest);
        readJson();
        pinMode(TS_ACTIVITY_LED_PIN, OUTPUT);
    }
}