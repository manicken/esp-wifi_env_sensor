
#include "REGO600.h"

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
    if (requestIndex < requestCount) SendNextRequest();
    else {
        // we are done rx whole LCD, send data back to whatever requested it
        if (actionDoneDestination == ActionDoneDestination::HttpReq) {
            if (pendingRequest == nullptr) return;
            String jsonStr = "{";
            jsonStr += "\"" + String(requests[0].text) + "\":\"" + String(lcdData,20) + "\",";
            jsonStr += "\"" + String(requests[1].text) + "\":\"" + String(&lcdData[20],20) + "\",";
            jsonStr += "\"" + String(requests[2].text) + "\":\"" + String(&lcdData[40],20) + "\",";
            jsonStr += "\"" + String(requests[3].text) + "\":\"" + String(&lcdData[60],20) + "\""; //charArrayToHex(String(&lcdData[60],20).c_str(),20) + "\"";
            jsonStr += "}";
            pendingRequest->send(200, "application/json; charset=windows-1252", jsonStr);
            //pendingRequest->send(200, "text/plain", String(lcdData, 80));
            pendingRequest=nullptr;
        }
        else if (actionDoneDestination == ActionDoneDestination::Websocket) {

        }
        else if (actionDoneDestination == ActionDoneDestination::Callback) {
            
        }
    }
}


const size_t RequestsAllTemperatures_Count = 9;
const REGO600::Request RequestsAllTemperatures[RequestsAllTemperatures_Count] = {
    {0x0209, "GT1"}, // Radiator return [GT1]
    {0x020A, "GT2"}, // Outdoor [GT2]
    {0x020B, "GT3"}, // Hot water [GT3]
    {0x020D, "GT5"}, // Room [GT5]
    {0x020E, "GT6"}, // Compressor [GT6]
    {0x020F, "GT8"}, // Heat fluid out [GT8]
    {0x0210, "GT9"}, // Heat fluid in [GT9]
    {0x0211, "GT10"}, // Cold fluid in [GT10]
    {0x0212, "GT11"}, // Cold fluid out [GT11]
};
void REGO600::BeginRetreiveAllTemperatures() { // maybe some callback function should be passed into this
    lastAction = Action::ReadTemperatures;
    currentExpectedRxLength = (uint8_t)CommandRxDataLenght::ReadRegister;
    requestCount = RequestsAllTemperatures_Count;
    requestIndex = 0;
    requests = RequestsAllTemperatures;
    uartTxBuffer[1] = (uint8_t)Command::ReadRegister;
    SendNextRequest();
}
void REGO600::RequestsAllTemperatures_Task() {
    // current temp index is calculated from requestIndex
    uint8_t tempIndex = requestIndex-1;
    temperatures[tempIndex] = GetValueFromUartRxBuff();
    if (requestIndex < requestCount) SendNextRequest();
    else {
        // we are done rx all temperatures, send data back to whatever requested it
        if (actionDoneDestination == ActionDoneDestination::HttpReq) {
            if (pendingRequest == nullptr) return;
            String jsonStr = "{";
            for (int i=0;i<RequestsAllTemperatures_Count;i++) {
                jsonStr += "\"" + String(requests[i].text) + "\":" + String((float)temperatures[i]/(float)10,1);
                if (i<RequestsAllTemperatures_Count-1) jsonStr += ",";
            }
            jsonStr += "}";
            pendingRequest->send(200, "application/json", jsonStr);
            pendingRequest=nullptr;
        }
        else if (actionDoneDestination == ActionDoneDestination::Websocket) {

        }
        else if (actionDoneDestination == ActionDoneDestination::Callback) {
            
        }
    }
}


const size_t RequestsAllStates_Count = 8;
const REGO600::Request RequestsAllStates[RequestsAllStates_Count] = {
    {0x01FD, "P3"}, // Ground loop pump [P3]
    {0x01FE, "COMP"}, // Compressor
    {0x01FF, "EH3"}, // Additional heat 3kW
    {0x0200, "EH6"}, // Additional heat 6kW
    {0x0203, "P1"}, // Radiator pump [P1]
    {0x0204, "P2"}, // Heat carrier pump [P2]
    {0x0205, "VXV"}, // Tree-way valve [VXV]
    {0x0206, "ALARM"}, // Cold fluid in [GT10]
};
void REGO600::BeginRetreiveAllStates() { // maybe some callback function should be passed into this
    lastAction = Action::ReadStates;
    currentExpectedRxLength = (uint8_t)CommandRxDataLenght::ReadRegister;
    requestCount = RequestsAllStates_Count;
    requestIndex = 0;
    requests = RequestsAllStates;
    uartTxBuffer[1] = (uint8_t)Command::ReadRegister;
    SendNextRequest();
}
void REGO600::RequestsAllStates_Task() {
    // current temp index is calculated from requestIndex
    uint8_t stateIndex = requestIndex-1;
    states[stateIndex] = GetValueFromUartRxBuff();
    if (requestIndex < requestCount) SendNextRequest();
    else {
        // we are done rx all temperatures, send data back to whatever requested it
        if (actionDoneDestination == ActionDoneDestination::HttpReq) {
            if (pendingRequest == nullptr) return;
            String jsonStr = "{";
            for (int i=0;i<RequestsAllStates_Count;i++) {
                jsonStr += "\"" + String(requests[i].text) + "\":" + String(states[i]==1?"true":"false");
                if (i<RequestsAllStates_Count-1) jsonStr += ",";
            }
            jsonStr += "}";
            pendingRequest->send(200, "application/json", jsonStr);
            pendingRequest=nullptr;
        }
        else if (actionDoneDestination == ActionDoneDestination::Websocket) {

        }
        else if (actionDoneDestination == ActionDoneDestination::Callback) {
            
        }
    }
}

REGO600::REGO600()
    : server(REGO600_WS_PORT), ws("/rego600ws"), UART2(Serial2) // Init UART2 as HardwareSerial(2)
{}

void REGO600::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;

        if (info->opcode == WS_BINARY) {
            lastAction = Action::WebSocketRaw;
            UART2.write(data, len);
        }
    }
}

void REGO600::setup() {
    uartTxBuffer[0] = 0x81; // allways 0x81, never to be changed
    UART2.begin(19200, SERIAL_8N1, 16, 17);  // Set correct RX/TX pins for UART2
    ws.onEvent([this](AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType t, void *a, uint8_t *d, size_t l) {
        this->onWsEvent(s, c, t, a, d, l);
    });

    server.addHandler(&ws);
    server.begin();
    server.on("/Temperatures", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->pendingRequest = request;
        this->actionDoneDestination = ActionDoneDestination::HttpReq;
        BeginRetreiveAllTemperatures();
    });
    server.on("/States", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->pendingRequest = request;
        this->actionDoneDestination = ActionDoneDestination::HttpReq;
        BeginRetreiveAllStates();
    });
    server.on("/LCD", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->pendingRequest = request;
        this->actionDoneDestination = ActionDoneDestination::HttpReq;
        BeginRetreiveWholeLCD();
    });
}

void REGO600::SendNextRequest() {
    uartRxBufferIndex = 0;
    SetRequestData(requests[requestIndex++]);
    CalcAndSetTxChecksum();
    UART2.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}

void REGO600::SetRequestData(REGO600::Request req) {
    uartTxBuffer[2] = (req.address >> 14) & 0x7F;
    uartTxBuffer[3] = (req.address >> 7) & 0x7F;
    uartTxBuffer[4] = req.address & 0x7F;
    uartTxBuffer[5] = 0x00; // allways zero
    uartTxBuffer[6] = 0x00; // allways zero
    uartTxBuffer[7] = 0x00; // allways zero
}

void REGO600::CalcAndSetTxChecksum() {
    uint8_t chksum = 0;
    for (int i=REGO600_UART_TX_CHKSUM_START_INDEX;i<REGO600_UART_TX_CHKSUM_END_INDEX;i++) {
        chksum ^= uartTxBuffer[i];
    }
    uartTxBuffer[REGO600_UART_TX_CHKSUM_END_INDEX] = chksum;
}

uint16_t REGO600::GetValueFromUartRxBuff() {
    return (uartRxBuffer[1] << 14) + (uartRxBuffer[2] << 7) + uartRxBuffer[3];
}

void REGO600::task_loop() {
    if (lastAction == Action::WebSocketRaw) {
        // Read all available UART data
        while (UART2.available()) {
            if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartRxBufferIndex++] = UART2.read();
                lastByteTime = millis(); // update last byte time
            }
        }

        // If buffer is not empty and timeout passed, send it
        if (uartRxBufferIndex > 0 && (millis() - lastByteTime > FLUSH_TIMEOUT_MS)) {
            ws.binaryAll(uartRxBuffer, uartRxBufferIndex);
            uartRxBufferIndex = 0;
        }
    }
    else {
        while (UART2.available()) {
            if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartRxBufferIndex++] = UART2.read();
                if (uartRxBufferIndex == currentExpectedRxLength) {
                    // RX is done
                    if (lastAction == Action::ReadWholeLCD) {
                        RequestsWholeLCD_Task();
                    }
                    else if (lastAction == Action::ReadTemperatures) {
                        RequestsAllTemperatures_Task();
                    }
                    else if (lastAction == Action::ReadStates) {
                        RequestsAllStates_Task();
                    }
                }
            }
        }
    }
}