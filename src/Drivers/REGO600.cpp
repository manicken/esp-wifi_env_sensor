
#include "REGO600.h"
//const size_t CmdVsResponseSizeTable_Count = 12;
const REGO600::CmdVsResponseSize CmdVsResponseSizeTable[] = {
    {REGO600::OpCodes::ReadFrontPanel,      5,  REGO600::Request::Type::Value}, // Read from front panel (keyboard+leds) {reg 09FF+xx}
    {REGO600::OpCodes::WriteFrontPanel,     1,  REGO600::Request::Type::WriteConfirm}, // Write to front panel (keyboard+leds) {reg 09FF+xx}
    {REGO600::OpCodes::ReadSystemRegister,  5,  REGO600::Request::Type::Value}, // Read from system register (heat curve, temperatures, devices) {reg 1345+xx}
    {REGO600::OpCodes::WriteSystemRegister, 1,  REGO600::Request::Type::WriteConfirm}, // Write into system register (heat curve, temperatures, devices) {reg 1345+xx}
    {REGO600::OpCodes::ReadTimerRegisters,  5,  REGO600::Request::Type::Value}, // Read from timer registers {reg 1B45+xx}
    {REGO600::OpCodes::WriteTimerRegisters, 1,  REGO600::Request::Type::WriteConfirm}, // Write into timer registers {reg 1B45+xx}
    {REGO600::OpCodes::ReadRegister_1B61,   5,  REGO600::Request::Type::Value}, // Read from register 1B61 {reg 1B61+xx}
    {REGO600::OpCodes::WriteRegister_1B61,  1,  REGO600::Request::Type::WriteConfirm}, // Write into register 1B61 {1B61+xx}
    {REGO600::OpCodes::ReadDisplay,         42, REGO600::Request::Type::Text}, // Read from display {0AC7+15h*xx}
    {REGO600::OpCodes::ReadLastError,       42, REGO600::Request::Type::ErrorLogItem}, // Read last error line [4100/00]
    {REGO600::OpCodes::ReadPrevError,       42, REGO600::Request::Type::ErrorLogItem}, // Read previous error line (prev from last reading) [4100/01]
    {REGO600::OpCodes::ReadRegoVersion,     5,  REGO600::Request::Type::Value} // Read rego version {constant 0258 = 600 ?Rego 600?}
};
const REGO600::CmdVsResponseSize* REGO600::getCmdInfo(uint8_t opcode) {

    for (const auto& entry : CmdVsResponseSizeTable) {
        if (static_cast<uint8_t>(entry.opcode) == opcode) {
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
        response.value = (buff[1] << 14) + (buff[2] << 7) + buff[3];

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

void REGO600::RefreshLoop_SendCurrent() {
    uartTxBuffer[1] = refreshLoopList[refreshLoopIndex]->opcode;
    SetRequestAddr(refreshLoopList[refreshLoopIndex]->address);
    CalcAndSetTxChecksum();
    auto info = getCmdInfo(uartTxBuffer[1]);
    currentExpectedRxLength = info->size;
    REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}

void REGO600::RefreshLoop_Restart() {
    refreshLoopCount = 0;
    RefreshLoop_SendCurrent();
}

void REGO600::RefreshLoop_Continue() {
    if (refreshLoopIndex != refreshLoopCount-1) {
        refreshLoopIndex++;
        RefreshLoop_SendCurrent();
    } else {
        // one loop done
        // exec some cb here, or set some flags
        
        waitForResponse = false; // wait until refresh time 
    }
}

void REGO600::loop() {
    if (waitForResponse == false) { //  here we just take care of any glitches and receive garbage data if any
        while (REGO600_UART_TO_USE.available()) { // to make sure that any garbage gets collected
            REGO600_UART_TO_USE.read();
        }
        if (mode != RequestMode::RefreshLoop) return;

        uint32_t now = millis();
        if (now - lastUpdateMs >= refreshTimeMs) {
            lastUpdateMs = millis();
            // need to start refreshLoop thingy again
            RefreshLoop_Restart();
        }
        return;
    }
#define REGO600_UART_RX_MAX_FAILSAFECOUNT 100
    uint32_t failsafeReadCount = 0;
    while (REGO600_UART_TO_USE.available() && failsafeReadCount++ < REGO600_UART_RX_MAX_FAILSAFECOUNT) {
        if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
            uartRxBuffer[uartRxBufferIndex++] = REGO600_UART_TO_USE.read();
            if (uartRxBufferIndex == currentExpectedRxLength) {
                // to make sure that any garbage gets collected
                failsafeReadCount = 0;
                while (REGO600_UART_TO_USE.available() && failsafeReadCount++ < REGO600_UART_RX_MAX_FAILSAFECOUNT) { 
                    REGO600_UART_TO_USE.read();
                }
                // RX is done
                if (mode == RequestMode::RefreshLoop) {
                    refreshLoopList[refreshLoopIndex]->SetFromBuffer(uartRxBuffer);
                    if (wantToManuallySend) {
                        wantToManuallySend = false;
                        if (DecodeManualRequest() == true)
                            return;
                    }
                    RefreshLoop_Continue();

                } else if (mode == RequestMode::Lcd) {
                    for (int bi=1,ti=0;ti<20;bi+=2,ti++) {
                        readLCD_Text[ti+readLCD_RowIndex*20] = uartRxBuffer[bi]*16 + uartRxBuffer[bi];
                    }
                    if (readLCD_RowIndex == 3) { // this was the last row
                        mode = RequestMode::RefreshLoop;
                        manuallyModeReq = RequestMode::RefreshLoop;
                        // execute a callback here

                        RefreshLoop_Continue();
                    } else {
                        readLCD_RowIndex++;
                        uartTxBuffer[4] = readLCD_RowIndex;
                        uartTxBuffer[8] = readLCD_RowIndex;
                        REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
                    }
                } else if (mode == RequestMode::FrontPanelLeds) {
                    if (uartRxBuffer[3] == 0x01) readFrontPanelLeds_Data |= 0x01;
                    if (readFrontPanelLedsIndex != 4) {
                        readFrontPanelLedsIndex++;
                        readFrontPanelLeds_Data <<= 1; // shift data to the right
                        uartTxBuffer[4] = readFrontPanelLedsIndex + 0x12;
                        uartTxBuffer[8] = readFrontPanelLedsIndex + 0x12;
                        REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
                    } else {
                        mode = RequestMode::RefreshLoop;
                        manuallyModeReq = RequestMode::RefreshLoop;
                        // execute callback here with contents of readFrontPanelLeds_Data (note we can use (void*) together with type info to send any kind of data pointers)
                        
                        RefreshLoop_Continue();
                    }
                } else if (mode == RequestMode::OneTime) {
                    manuallyRequest->SetFromBuffer(uartRxBuffer);
                    mode = RequestMode::RefreshLoop;
                    manuallyModeReq = RequestMode::RefreshLoop;

                    // execute a callback here with manuallyRequest as param
                    // now safe to delete
                    delete manuallyRequest;
                    manuallyRequest = nullptr;
                    
                    RefreshLoop_Continue();
                }

                
                return; // now we can return here
            }
        } else {
            waitForResponse = false; // to make the remaining data reads faster, if any 
            REGO600_UART_TO_USE.read(); // must read remaining to get out of loop (and most important empty the buffer)
            GlobalLogger.Error(F("REGO600 - uartRxBuffer full"));
            //if (this->ws.count() > 0) this->ws.textAll("{\"error\":\"uartRxBuffer full\"}\n");
        }
    }
    if (failsafeReadCount == REGO600_UART_RX_MAX_FAILSAFECOUNT) {
        GlobalLogger.Error(F("REGO600 - read failsafe overflow"));
    }
}

bool REGO600::DecodeManualRequest() {
    if (manuallyModeReq == RequestMode::Lcd) {
        uartTxBuffer[1] = static_cast<uint8_t>(OpCodes::ReadDisplay);
        readLCD_RowIndex = 0;
        SetRequestAddr(0x00);
        uartTxBuffer[8] = 0x00;
        if (readLCD_Text == nullptr) // initialize it if this is the first use
            readLCD_Text = new char[20*4+1]();
        

    } else if (manuallyModeReq == RequestMode::FrontPanelLeds) {
        readFrontPanelLeds_Data = 0x00;
        uartTxBuffer[1] = static_cast<uint8_t>(OpCodes::ReadFrontPanel);
        readFrontPanelLedsIndex = 0;
        SetRequestAddr(0x12);
        CalcAndSetTxChecksum();
        
    } else if (manuallyModeReq == RequestMode::OneTime && manuallyRequest != nullptr) {
        uartTxBuffer[1] = manuallyRequest->opcode;
        SetRequestAddr(manuallyRequest->address);
        CalcAndSetTxChecksum();
        
    } else {
        return false;
    }
    mode = manuallyModeReq;
    auto info = getCmdInfo(uartTxBuffer[1]);
    currentExpectedRxLength = info->size;
    REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
    return true;
}
/*
void REGO600::SendNextRequest() {
    
    SendReq(refreshLoopList[refreshLoopIndex++]->address);
}
*/
void REGO600::SendReq(uint16_t address) {
    uartRxBufferIndex = 0;
    SetRequestAddr(address);
    CalcAndSetTxChecksum();
    REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}

void REGO600::Send(uint16_t address, uint16_t data) {
    uartRxBufferIndex = 0;
    SetRequestAddr(address);
    SetRequestData(data);
    CalcAndSetTxChecksum();
    REGO600_UART_TO_USE.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}
/*
void REGO600::StartSendOneRegisterReadRequest(uint16_t address) {
    currentExpectedRxLength = (uint8_t)CommandRxDataLenght::ReadRegister;
    uartTxBuffer[1] = (uint8_t)Command::ReadRegister;
    SendReq(address);
}
*/
void REGO600::SetRequestAddr(uint16_t address) {
    uartTxBuffer[2] = (address >> 14) & 0x7F;
    uartTxBuffer[3] = (address >> 7) & 0x7F;
    uartTxBuffer[4] = address & 0x7F;
    uartTxBuffer[5] = 0x00; // allways zero unless set
    uartTxBuffer[6] = 0x00; // allways zero unless set
    uartTxBuffer[7] = 0x00; // allways zero unless set
}

void REGO600::SetRequestData(uint16_t data) {
    uartTxBuffer[5] = (data >> 14) & 0x7F;
    uartTxBuffer[6] = (data >> 7) & 0x7F;
    uartTxBuffer[7] = data & 0x7F;
}

void REGO600::CalcAndSetTxChecksum() {
    uint8_t chksum = 0;
    for (int i=2;i<8;i++) {
        chksum ^= uartTxBuffer[i];
    }
    uartTxBuffer[8] = chksum;
}

uint16_t REGO600::GetValueFromUartRxBuff() {
    return (uartRxBuffer[1] << 14) + (uartRxBuffer[2] << 7) + uartRxBuffer[3];
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