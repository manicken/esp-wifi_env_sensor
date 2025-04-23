
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define REGO600_UART_TX_CHKSUM_START_INDEX 2
#define REGO600_UART_TX_BUFFER_SIZE 9 // allways 9 in size
#define REGO600_UART_TX_CHKSUM_END_INDEX (REGO600_UART_TX_BUFFER_SIZE-1)
#define REGO600_UART_RX_BUFFER_SIZE 42 // 42 is the max rx size from REGO600
#define REGO600_WS_PORT 8080


class REGO600 {
public:
    REGO600();
    void setup();
    void task_loop();
    void BeginRetreiveWholeLCD();

    enum class Command : int16_t{
        None = -1,
        ReadPanel = 0x00,
        WritePanel = 0x01,
        ReadRegister = 0x02,
        ReadLcd = 0x20,
        ReadLastError = 0x40,
        ReadPrevError = 0x44
    };
    enum class CommandRxDataLenght : uint8_t {
        ReadPanel = 5,
        WritePanel = 1,
        ReadRegister = 5,
        ReadLcd = 42,
        ReadLastError = 42,
        ReadPrevError = 42
    };
    struct Request {
        uint16_t address;
        uint16_t data;
    };
private:
    
    uint8_t uartTxBuffer[REGO600_UART_TX_BUFFER_SIZE]; 
    uint8_t uartRxBuffer[REGO600_UART_RX_BUFFER_SIZE];
    size_t uartBufferIndex = 0;
    unsigned long lastByteTime = 0;
    const unsigned long FLUSH_TIMEOUT_MS = 50; // flush after 50ms idle

    Command lastCommand = Command::None;
    uint8_t currentExpectedRxLength = 0;
    //size_t currentRxCount = 0;
    const Request *requests; // contains a list of reg adresses to read
    uint8_t requestIndex = 0;
    uint8_t requestCount = 0;

    char lcdData[20*4];
    uint16_t temperatures[9];

    AsyncWebServer server;
    AsyncWebSocket ws;
    HardwareSerial& UART2;
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void SetRequest(REGO600::Request req);
    void CalcAndSetTxChecksum();
};
const REGO600::Request RequestsWholeLCD[];
const REGO600::Request RequestsAllTemperatures[];

extern const size_t RequestsWholeLCD_Count;
extern const size_t RequestsAllTemperatures_Count;
