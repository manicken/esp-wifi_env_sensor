
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define REGO600_UART_BUFFER_SIZE 256
#define REGO600_WS_PORT 8080


class REGO600 {
public:
    REGO600();
    void setup();
    void task_loop();

    enum Command {
        None = -1,
        ReadPanel = 0x00,
        WritePanel = 0x01,
        ReadRegister = 0x02,
        ReadLcd = 0x20,
        ReadLastError = 0x40,
        ReadPrevError = 0x44
    };
private:
    
    uint8_t uartBuffer[REGO600_UART_BUFFER_SIZE];
    size_t uartBufferIndex = 0;
    unsigned long lastByteTime = 0;
    const unsigned long FLUSH_TIMEOUT_MS = 50; // flush after 50ms idle

    Command lastCommand = Command::None;
    uint8_t currentExpectedRxLength = 0;
    uint8_t currentRxCount = 0;

    AsyncWebServer server;
    AsyncWebSocket ws;
    HardwareSerial& UART2;
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};

