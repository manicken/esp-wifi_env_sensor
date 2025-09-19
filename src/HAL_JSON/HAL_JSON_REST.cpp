/*

  <one line to give the program's name and a brief idea of what it does.>

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "HAL_JSON_REST.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

namespace HAL_JSON {

    AsyncWebServer* REST::asyncWebserver = nullptr;

    void DefaultHandler(AsyncWebServerRequest *request) {
        const char* urlStr = request->url().c_str();

        if (urlStr == nullptr || *urlStr == '\0') {
            request->send(200, "application/json", "{\"error\":\"urlEmpty\"}");
            return;
        }
        if (*(urlStr + 1) == '\0') {
            request->send(200, "application/json", "{\"error\":\"emptyPath\"}");
            return;
        }

        //ZeroCopyString zcUrl(urlStr+1); // +1 removes the leading /
        // Capture request in lambda for later response
        
        CommandExecutor_LOCK_QUEUE();
        CommandExecutor::g_pending.push({std::string(urlStr + 1), 
            [request](const std::string& response) {
                if (request->client()->connected()) {
                    request->send(200, "application/json", response.c_str());
                }
            }
        });
        CommandExecutor_UNLOCK_QUEUE();

        /*bool accepted = CommandExecutor::execute(zcUrl, 
            [request](const std::string& message) {
                // NOTE: this runs later when the command finishes
                if (request->client()->connected()) {
                    request->send(200, "application/json", message.c_str());
                }
            }
        );

        if (!accepted) {
            GlobalLogger.printAllLogs(Serial, false); // TODO make this print back to request client
            //request->send(200, "application/json", "{\"error\":\"commandNotAccepted\"}");
        }*/
/*
        std::string message;
        bool success = CommandExecutor::execute(zcUrl, message);
        // do something when success == false
        // maybe add last entry in GlobalLogger
        if (success == false) {
            GlobalLogger.printAllLogs(Serial, false); // TODO make this print back to request client
        }
        request->send(200, "application/json", message.c_str());*/
    }

    void REST::setupRest() {
        asyncWebserver = new AsyncWebServer(82);
        asyncWebserver->onNotFound(DefaultHandler);
        asyncWebserver->begin();
    }
}
