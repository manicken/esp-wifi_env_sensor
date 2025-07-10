
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "../Support/ConvertHelper.h"

#define REGO600_UART_TX_CHKSUM_START_INDEX 2
#define REGO600_UART_TX_BUFFER_SIZE 9 // allways 9 in size
#define REGO600_UART_TX_CHKSUM_END_INDEX (REGO600_UART_TX_BUFFER_SIZE-1)
#define REGO600_UART_RX_BUFFER_SIZE 42 // 42 is the max rx size from REGO600

#if defined(ESP32)
#define REGO600_UART_TO_USE Serial2
#elif defined(ESP8266)
#define REGO600_UART_TO_USE Serial
#endif

class REGO600 {
public:
    
    struct Request {
        enum class Type {
            Value,
            Text,
            ErrorLogItem,
            WriteConfirm
        };

        uint16_t address;
        Type type = Type::Value;

        union {
            uint32_t value;
            char* text;
        } response;

        Request() = delete;
        Request(uint16_t address, Type type);
        void SetFromBuffer(uint8_t* buff);

        ~Request();

        // prevent accidental copies/moves
        Request(const Request&) = delete;
        Request& operator=(const Request&) = delete;
        Request(Request&&) = delete;
        Request& operator=(Request&&) = delete;
    };
    struct CmdVsResponseSize {
        uint32_t opcode;
        uint32_t size;
        Request::Type type;
    };

    /** theese are the states the main task can be in */
    enum class RequestMode {
        RefreshLoop,
        Lcd,
        FrontPanelLeds,
        OneTime
    };

    REGO600();
    void setup(int8_t rxPin, int8_t txPin);
    void loop();
    static const CmdVsResponseSize* getCmdInfo(uint8_t opcode);
    
private:
    
    uint8_t uartTxBuffer[REGO600_UART_TX_BUFFER_SIZE]; 
    uint8_t uartRxBuffer[REGO600_UART_RX_BUFFER_SIZE];
    size_t uartRxBufferIndex = 0;
    size_t currentExpectedRxLength = 0;
    
    Request** refreshLoopList;
    int refreshLoopCount = 0;
    int refreshLoopIndex = 0;

    RequestMode mode = RequestMode::RefreshLoop;
    /** set to true when currently waiting for data to be received */
    bool waitForResponse = false;
    /** set to true when manual interventioon need to be executed */
    bool wantToManuallySend = false;
};
