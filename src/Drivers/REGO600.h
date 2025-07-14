
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

// 240-270mS is the measured range so 300mS would be safe
// note this is while the REGO600 is in "on" state
// if it's turned off by the front panel button the read time is ~10x less
#define REGO600_DRIVER_READ_REGISTER_TIME_MS_ON_STATE 300
#define REGO600_DRIVER_READ_REGISTER_TIME_MS_OFF_STATE 30

namespace Drivers {

    class REGO600 {
    public:
        
        

        // theese are the only available op-codes (verified by rom-dump)
        enum class OpCodes : uint8_t {
            ReadFrontPanel = 0x00,
            WriteFrontPanel = 0x01,
            ReadSystemRegister = 0x02,
            WriteSystemRegister = 0x03,
            ReadTimerRegisters = 0x04,
            WriteTimerRegisters = 0x05,
            ReadRegister_1B61 = 0x06,
            WriteRegister_1B61 = 0x07,
            ReadDisplay = 0x20,
            ReadLastError = 0x40,
            ReadPrevError = 0x42,
            ReadRegoVersion = 0x7F
        };
        
        struct Request {
            enum class Type {
                Value,
                Text,
                ErrorLogItem,
                WriteConfirm
            };

            const uint32_t opcode;
            const uint16_t address;
            const Type type = Type::Value;

            union Response { // type name this so that it can be passed
                uint32_t* value;
                char* text;
            } response;

            Request() = delete;
            /**  */
            Request(uint32_t opcode, uint16_t address, Type type);
            /** externalValue is a non-owning pointer to external data. 
             * Do not delete; must remain valid during Request's lifetime.
             */
            Request(uint32_t opcode, uint16_t address, uint32_t& externalValue);
            void SetFromBuffer(uint8_t* buff);

            ~Request();

            // prevent accidental copies/moves
            Request(const Request&) = delete;
            Request& operator=(const Request&) = delete;
            Request(Request&&) = delete;
            Request& operator=(Request&&) = delete;
        };
        

        struct CmdVsResponseSize {
            OpCodes opcode;
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

        using RequestCallback = void (*)(void*, RequestMode);


        REGO600() = delete;
        REGO600(REGO600&) = delete;
        REGO600(int8_t rxPin, int8_t txPin, Request** refreshLoopList, int refreshLoopCount, uint32_t refreshTimeMs);
        ~REGO600();
        void begin();
        void loop();
        /** please note that this uses std::unique_ptr 
         * to ensure that the created object (Request) is moved into this function
         * and to make sure that it can be deleted internally */
        void OneTimeRequest(std::unique_ptr<Request> req, RequestCallback cb);
        void RequestWholeLCD(RequestCallback cb);
        void RequestFrontPanelLeds(RequestCallback cb);

        static const CmdVsResponseSize* getCmdInfo(uint8_t opcode);
        /** this is a "latching flag"/"one-shot flag", i.e. if read and was true it automatically resets the internal flag to false */
        bool RefreshLoopDone();
    private:
        
        uint8_t uartTxBuffer[REGO600_UART_TX_BUFFER_SIZE]; 
        uint8_t uartRxBuffer[REGO600_UART_RX_BUFFER_SIZE];
        size_t uartRxBufferIndex = 0;
        size_t currentExpectedRxLength = 0;
        
        uint32_t refreshTimeMs = 5000; // this needs to calculated depending on how many items in refreshLoopList (max items is 17 -> 17*0.2 = 3.4 sec) but also what the json cfg have
        uint32_t lastUpdateMs = 0;
        Request* const* refreshLoopList; // a const pointer to a list of mutable Request object pointers
        const int refreshLoopCount;
        int refreshLoopIndex = 0;
        bool refreshLoopDone = false;

        /** make this dynamicaly allocated to save memory if not used */
        char* readLCD_Text = nullptr;
        int readLCD_RowIndex = 0;
        uint8_t readFrontPanelLeds_Data = 0x00; // store all led status only in a byte
        int readFrontPanelLedsIndex = 0;


        RequestMode mode = RequestMode::RefreshLoop;
        /** set to true when currently waiting for data to be received */
        bool requestInProgress = false;
        
        /** set to true when manual interventioon need to be executed */
        bool manualRequest_Pending = false;
        RequestMode manualRequest_Mode = RequestMode::RefreshLoop; // default value
        std::unique_ptr<Request> manualRequest = nullptr; // this is used when simple values need to be read/written i.e. when manuallyModeReq == RequestMode::OneTime
        RequestCallback manualRequest_Callback = nullptr;
        void ManualRequest_Schedule(RequestMode reqMode);
        bool ManualRequest_PrepareAndSend();

        void SetRequestAddr(uint16_t address);
        void SetRequestData(uint16_t data);
        void SendReq(uint16_t address);
        void Send(uint16_t address, uint16_t data);
        //void SendNextRequest();
        void StartSendOneRegisterReadRequest(uint16_t address);
        void CalcAndSetTxChecksum();
        uint16_t GetValueFromUartRxBuff();

        void RefreshLoop_Restart();
        void RefreshLoop_SendCurrent();
        void RefreshLoop_Continue();

        static void ClearUARTRxBuffer(HardwareSerial& uart, size_t maxDrains = 100);
    };
}