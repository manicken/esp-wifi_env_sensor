
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
            if (success == false) {
                GlobalLogger.printAllLogs(Serial, false); // TODO make this print back to request client
            }
            request->send(200, "application/json", message.c_str());
        });
        asyncWebserver->begin();
    }
}
