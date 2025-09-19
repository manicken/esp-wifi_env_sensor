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

#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define UART2WS_BUFFER_SIZE 256
#define UART2WS_PORT 8080
#if defined(ESP32)
#define UART2WS_UART_TO_USE Serial2
#elif defined(ESP8266)
#define UART2WS_UART_TO_USE Serial
#endif

class UART2websocket {
public:
    UART2websocket();
    void setup();
    void task_loop();

private:
    
    uint8_t uartBuffer[UART2WS_BUFFER_SIZE];
    size_t uartBufferIndex = 0;
    unsigned long lastByteTime = 0;
    const unsigned long FLUSH_TIMEOUT_MS = 50; // flush after 50ms idle

    AsyncWebServer server;
    AsyncWebSocket ws;
    HardwareSerial& UART;
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};

