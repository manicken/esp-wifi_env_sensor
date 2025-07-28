
#include "HAL_JSON_REST.h"

namespace HAL_JSON {

    AsyncWebServer* REST::asyncWebserver = nullptr;

    void REST::setup() {
        asyncWebserver = new AsyncWebServer(HAL_JSON_REST_API_PORT);
        asyncWebserver->onNotFound([](AsyncWebServerRequest *request){

            const char* urlStr = request->url().c_str();

            if (urlStr == nullptr || *urlStr == '\0') {
                request->send(200, "application/json", "{\"error\":\"urlEmpty\"}");
                return;
            }
            if (*(urlStr + 1) == '\0') {
                request->send(200, "application/json", "{\"error\":\"emptyPath\"}");
                return;
            }

            ZeroCopyString zcUrl(urlStr+1); // +1 removes the leading /
            std::string message;
            bool success = CommandExecutor::execute(zcUrl, message);
            // do something when success == false
            // maybe add last entry in GlobalLogger
            request->send(200, "application/json", message.c_str());
        });
        asyncWebserver->begin();
    }
}


/*
        asyncWebserver->on(HAL_JSON_REST_API_WRITE_URL "*", HTTP_ANY, handleWriteOrRead);
        asyncWebserver->on(HAL_JSON_REST_API_READ_URL "*", HTTP_ANY, handleWriteOrRead);

        asyncWebserver->on(HAL_JSON_URL_RELOAD_JSON, HTTP_ANY, [](AsyncWebServerRequest *request){
            if (Manager::ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()) == false) {
                GlobalLogger.printAllLogs(Serial, false); // TODO make this print back to request client

                const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                String message = "{\"error\":\""+lastEntry.MessageToString()+"\"}";
                request->send(200, "application/json", message);
                
            }
            else
            {
                request->send(200, "application/json", "{\"info\":\"JSON reload ok\"}");
                Manager::begin(); // call the begin function on all loaded hal devices
            }
        });
        asyncWebserver->on(HAL_JSON_URL_PRINT_DEVICES, HTTP_ANY, [](AsyncWebServerRequest* request){
            request->send(200, "application/json", Manager::ToString().c_str());
        });

        asyncWebserver->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
            String message = "HAL JSON Manager - REST API<br>";
            message += "<a href=\"/<cmd>/<type>/<uid>/<optional_value>\">Device command</a><br>";
            request->send(200, "text/html", message);
        });
        */