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

