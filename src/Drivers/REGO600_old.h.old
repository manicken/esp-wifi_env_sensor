
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "../Support/ConvertHelper.h"

#define REGO600_UART_TX_CHKSUM_START_INDEX 2
#define REGO600_UART_TX_BUFFER_SIZE 9 // allways 9 in size
#define REGO600_UART_TX_CHKSUM_END_INDEX (REGO600_UART_TX_BUFFER_SIZE-1)
#define REGO600_UART_RX_BUFFER_SIZE 42 // 42 is the max rx size from REGO600
#define REGO600_WS_PORT 8080

#if defined(ESP32)
#define REGO600_UART_TO_USE Serial2
#elif defined(ESP8266)
#define REGO600_UART_TO_USE Serial
#endif

class REGO600 {
public:
    REGO600();
    void setup();
    void task_loop();
    void BeginRetreiveWholeLCD();
    void BeginRetreiveAllTemperatures();
    void BeginRetreiveAllStates();
    

    enum class Command : uint16_t{
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
    enum class Action {
        NotSet = 0,
        WebSocketRaw = 1,
        ReadWholePanel = 2,
        ReadWholeLCD = 3,
        ReadTemperatures = 4,
        ReadStates = 5,
        ReadTemperature = 6,
        ReadState = 7
    };
    struct Request {
        uint16_t address;
        const char *text;
    };
private:
    
    uint8_t uartTxBuffer[REGO600_UART_TX_BUFFER_SIZE]; 
    uint8_t uartRxBuffer[REGO600_UART_RX_BUFFER_SIZE];
    size_t uartRxBufferIndex = 0;
    unsigned long lastByteTime = 0;
    const unsigned long FLUSH_TIMEOUT_MS = 50; // flush after 50ms idle

    Action lastAction = Action::WebSocketRaw;
    //ActionDoneDestination actionDoneDestination = ActionDoneDestination::NotSet;

    uint8_t currentExpectedRxLength = 0;
    //size_t currentRxCount = 0;
    const Request *requests; // contains a list of reg adresses to read
    uint8_t requestIndex = 0;
    uint8_t requestCount = 0;

    std::function<void(String)> onUartQueryComplete;

    char lcdData[20*4];
    uint16_t temperatures[9];
    uint16_t states[8];
    unsigned long startTimeMs = 0; // used for debuggin times

    AsyncWebServer server;
    AsyncWebSocket ws;
    HardwareSerial& UART;
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void SetRequestAddr(uint16_t address);
    void SetRequestData(uint16_t data);
    void SendReq(uint16_t address);
    void Send(uint16_t address, uint16_t data);
    void SendNextRequest();
    void StartSendOneRegisterReadRequest(uint16_t address);
    void CalcAndSetTxChecksum();
    uint16_t GetValueFromUartRxBuff();

    void RequestsWholeLCD_Task();
    void RequestsAllTemperatures_Task();
    void RequestsAllStates_Task();

    bool GetTemperatureRegisterAddrFromName(const char *name, uint32_t *addr);
    bool GetStatusRegisterAddrFromName(const char *name, uint32_t *addr);

    void RequestOneTemperature_Task();
    void RequestOneState_Task();
};
extern const REGO600::Request RequestsWholeLCD[];
extern const REGO600::Request RequestsAllTemperatures[];

extern const size_t RequestsWholeLCD_Count;
extern const size_t RequestsAllTemperatures_Count;
