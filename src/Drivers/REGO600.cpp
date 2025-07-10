
#include "REGO600.h"
//const size_t CmdVsResponseSizeTable_Count = 12;
const REGO600::CmdVsResponseSize CmdVsResponseSizeTable[] = {
    {0x00, 5, REGO600::Request::Type::Value}, // Read from front panel (keyboard+leds) {reg 09FF+xx}
    {0x01, 1, REGO600::Request::Type::WriteConfirm}, // Write to front panel (keyboard+leds) {reg 09FF+xx}
    {0x02, 5, REGO600::Request::Type::Value}, // Read from system register (heat curve, temperatures, devices) {reg 1345+xx}
    {0x03, 1, REGO600::Request::Type::WriteConfirm}, // Write into system register (heat curve, temperatures, devices) {reg 1345+xx}
    {0x04, 5, REGO600::Request::Type::Value}, // Read from timer registers {reg 1B45+xx}
    {0x05, 1, REGO600::Request::Type::WriteConfirm}, // Write into timer registers {reg 1B45+xx}
    {0x06, 5, REGO600::Request::Type::Value}, // Read from register 1B61 {reg 1B61+xx}
    {0x07, 1, REGO600::Request::Type::WriteConfirm}, // Write into register 1B61 {1B61+xx}
    {0x20, 42, REGO600::Request::Type::Text}, // Read from display {0AC7+15h*xx}
    {0x40, 42, REGO600::Request::Type::Text}, // Read last error line [4100/00]
    {0x42, 42, REGO600::Request::Type::Text}, // Read previous error line (prev from last reading) [4100/01]
    {0x7F, 5, REGO600::Request::Type::Value} // Read rego version {constant 0258 = 600 ?Rego 600?}
};
const REGO600::CmdVsResponseSize* REGO600::getCmdInfo(uint8_t opcode) {
    for (const auto& entry : CmdVsResponseSizeTable) {
        if (entry.opcode == opcode) {
            return &entry;
        }
    }
    return nullptr; // Not found
}

REGO600::Request::Request(uint16_t address, Type type) : type(type), address(address) {
    if (type == Type::Text) {
        response.text = new char[21](); 
    }
    else if (type == Type::Value) {
        response.value = 0;
    }
    else if (type == Type::ErrorLogItem) {
        response.text = new char[20](); // 3 digit error code + space + 6 char date + space + 8 char time + null terminator
    }
}
void REGO600::Request::SetFromBuffer(uint8_t* buff) {
    if (type == Type::Value) {

    } else if (type == Type::Text) {
        for (int bi=1,ti=0;ti<20;bi+=2,ti++) {
            response.text[ti] = buff[bi]*16 + buff[bi];
        }
    } else if (type == Type::ErrorLogItem) {
        
        uint32_t code = buff[1]*16 + buff[2];
        response.text[0] = (code / 100)+0x30;
        code %= 100;
        response.text[1] = (code / 10)+0x30;
        code %= 10;
        response.text[2] = (code)+0x30;
        response.text[3] = 0x20; // space
        for (int bi=3,ti=4;ti<20;bi+=2,ti++) {
            response.text[ti] = buff[bi]*16 + buff[bi];
        }
    } // there are currently no more types right now
}
REGO600::Request::~Request() {
    if (type == Type::Text && response.text != nullptr) {
        delete[] response.text;
    }
}

void REGO600::setup(int8_t rxPin, int8_t txPin) {
    uartTxBuffer[0] = 0x81; // constant
    #if defined(ESP32)
    REGO600_UART_TO_USE.begin(19200, SERIAL_8N1, rxPin, txPin); // Set correct RX/TX pins for UART
#elif defined(ESP8266)
    REGO600_UART_TO_USE.begin(19200, SERIAL_8N1); // note on esp8266 pins are not reconfigurable
#endif
}

void REGO600::loop() {
    if (waitForResponse == false) { //  here we just take care of any glitches and receive garbage data if any
        while (REGO600_UART_TO_USE.available()) { // to make sure that any garbage gets collected
            REGO600_UART_TO_USE.read();
        }
        return;
    }

    while (REGO600_UART_TO_USE.available()) {
        if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
            uartRxBuffer[uartRxBufferIndex++] = REGO600_UART_TO_USE.read();
            if (uartRxBufferIndex == currentExpectedRxLength) {
                // RX is done
                if (mode == RequestMode::RefreshLoop) {
                    refreshLoopList[refreshLoopIndex].
                } else if (mode == RequestMode::Lcd) {

                } else if (mode == RequestMode::FrontPanelLeds) {

                } else if (mode == RequestMode::OneTime) {

                } 
            }
        } else {
            REGO600_UART_TO_USE.read(); // must read remaining to get out of loop
            GlobalLogger.Error(F("REGO600 - uartRxBuffer full"));
            //if (this->ws.count() > 0) this->ws.textAll("{\"error\":\"uartRxBuffer full\"}\n");
        }
    }
}

/*

const size_t RequestsWholeLCD_Count = 4;
const REGO600::Request RequestsWholeLCD[RequestsWholeLCD_Count] = {
    {0x0000, "1"},
    {0x0001, "2"},
    {0x0002, "3"},
    {0x0003, "4"}
};
String charArrayToHex(const char* data, size_t length) {
    String hexStr = "";
    for (size_t i = 0; i < length; ++i) {
        if (data[i] < 16) hexStr += "0";  // Add leading zero for single-digit hex
        hexStr += String((uint8_t)data[i], HEX);
        hexStr += " ";
    }
    return hexStr;
}
void REGO600::BeginRetreiveWholeLCD() { // maybe some callback function should be passed into this
    lastAction = Action::ReadWholeLCD;
    currentExpectedRxLength = (uint8_t)CommandRxDataLenght::ReadLcd;
    requestCount = RequestsWholeLCD_Count;
    requestIndex = 0;
    requests = RequestsWholeLCD;
    uartTxBuffer[1] = (uint8_t)Command::ReadLcd;
    SendNextRequest();
}
void REGO600::RequestsWholeLCD_Task() {
    // current line of lcd is calculated from requestIndex
    uint8_t lcdRowIndex = requestIndex-1;
    uint8_t lcdColIndex = 0;
    for (int i=1;i<41;i+=2) {
        lcdData[lcdRowIndex*20+lcdColIndex++] = uartRxBuffer[i]*16 + uartRxBuffer[i+1];
    }
    if (requestIndex < requestCount) { delay(100); SendNextRequest(); }
    else {
        // we are done rx whole LCD, send data back to whatever requested it
        lastAction = Action::NotSet;
        if (onUartQueryComplete == nullptr) return;
        
        String jsonStr = "{";
        jsonStr += "\""; jsonStr += requests[0].text; jsonStr += "\":\""; jsonStr.concat(lcdData,20); jsonStr += "\",";
        jsonStr += "\""; jsonStr += requests[1].text; jsonStr += "\":\""; jsonStr.concat(&lcdData[20],20); jsonStr += "\",";
        jsonStr += "\""; jsonStr += requests[2].text; jsonStr += "\":\""; jsonStr.concat(&lcdData[40],20); jsonStr += "\",";
        jsonStr += "\""; jsonStr += requests[3].text; jsonStr += "\":\""; jsonStr.concat(&lcdData[60],20); jsonStr += "\"";
        jsonStr += "}";
        onUartQueryComplete(jsonStr);
    }
}
    */