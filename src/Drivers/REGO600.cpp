
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
bool REGO600::GetTemperatureRegisterAddrFromName(const char *name, uint32_t *addr) {
    if (name == nullptr) return false;
    if (addr == nullptr) return false;
    for (size_t i=0;i<RequestsAllTemperatures_Count;i++){
        if (strcmp(RequestsAllTemperatures[i].text, name) == 0) {
            *addr = RequestsAllTemperatures[i].address;
            return true;
        }
    }
    return false;
}
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
    if (requestIndex < requestCount) { delay(10); SendNextRequest(); }
    else {
        lastAction = Action::NotSet;
        // we are done rx all temperatures, send data back to whatever requested it
        if (onUartQueryComplete == nullptr) return;

        String jsonStr = "{";
        for (size_t i=0;i<RequestsAllTemperatures_Count;i++) {
            jsonStr += "\"" + String(requests[i].text) + "\":" + String((float)temperatures[i]/(float)10,1);
            if (i<RequestsAllTemperatures_Count-1) jsonStr += ",";
        }
        jsonStr += "}";
        onUartQueryComplete(jsonStr);
    }
}
void REGO600::RequestOneTemperature_Task() {
    lastAction = Action::NotSet;
    if (onUartQueryComplete == nullptr) return;
    uint16_t temp = GetValueFromUartRxBuff();
    
    String jsonStr = "{\"value\":" + String((float)temp/(float)10,1) + "}";
    onUartQueryComplete(jsonStr);
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
bool REGO600::GetStatusRegisterAddrFromName(const char *name, uint32_t *addr) {
    if (name == nullptr) return false;
    if (addr == nullptr) return false;
    for (size_t i=0;i<RequestsAllStates_Count;i++){
        if (strcmp(RequestsAllStates[i].text, name) == 0) {
            *addr = RequestsAllStates[i].address;
            return true;
        }
    }
    return false;
}
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
    if (requestIndex < requestCount) { delay(10); SendNextRequest(); }
    else {
        lastAction = Action::NotSet;
        // we are done rx all temperatures, send data back to whatever requested it
        if (onUartQueryComplete == nullptr) return; // do nothing

        String jsonStr = "{";
        for (size_t i=0;i<RequestsAllStates_Count;i++) {
            jsonStr += "\"" + String(requests[i].text) + "\":" + String(states[i]==1?"true":"false");
            if (i<RequestsAllStates_Count-1) jsonStr += ",";
        }
        jsonStr += "}";
        onUartQueryComplete(jsonStr);
    }
}
void REGO600::RequestOneState_Task() {
    lastAction = Action::NotSet;
    if (onUartQueryComplete == nullptr) return;
    uint16_t state = GetValueFromUartRxBuff();
    String jsonStr = "{\"value\":" + String((state==1)?"true":"false") + "}";
    onUartQueryComplete(jsonStr);
}

REGO600::REGO600()
    : server(REGO600_WS_PORT), ws("/rego600ws"), UART(REGO600_UART_TO_USE) // Init UART2 as HardwareSerial(2)
{}

String toHex(const char *data, size_t len) {
    String hex = "";
    for (size_t i = 0; i < len; i++) {
        if (data[i] < 16) hex += "0";  // Pad single digits
        hex += String(data[i], HEX);
        if (i<len-1) hex += ",";
    }
    return hex;
}

String convertISO88591toUTF8(const String &input) {
  String output = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    unsigned char c = input[i];
    if (c == 1) output += "\u2581";
    else if (c == 2) output += "\u2582";
    else if (c == 3) output += "\u2583";
    else if (c == 4) output += "\u2584";
    else if (c == 5) output += "\u2585";
    else if (c == 6) output += "\u2586";
    else if (c == 7) output += "\u2587";
    else if (c == 8) output += "\u2588";
    else if (c < 32) output += "{"+String(c,16)+"}";
    else if (c < 128) {
      output += (char)c;
    } else {
      output += (char)(0xC0 | (c >> 6));
      output += (char)(0x80 | (c & 0x3F));
    }
  }
  return output;
}

void REGO600::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        
        if (info->opcode == WS_TEXT) {
            lastAction = Action::NotSet;
            // Copy data to a null-terminated string
            String jsonString = String((char*)data).substring(0, len);
            
            // Allocate a JSON document (use ArduinoJson Assistant to size this correctly)
            DynamicJsonDocument doc(jsonString.length() * 2);
            DeserializationError error = deserializeJson(doc, jsonString);

            if (error) {
                Serial.print("JSON parse failed: ");
                Serial.println(error.c_str());
                // Send error message back to the same client
                String errMsg = String("{\"error\":\"JSON parse failed: ") + error.c_str() + "\"}";
                client->text(errMsg);  // Sends a WebSocket text message to the client
                return;
            }

            // Example: reading a key "command" from the JSON
            if (doc.containsKey("cmd")) {
                String command = doc["cmd"];
                Serial.println("Received command: " + command);
                // You can take action based on the command here
                if (command == "temperatures") {
                    onUartQueryComplete = [client,this](String jsonResponse) {
                        unsigned long totalTimeMs = millis()-this->startTimeMs;
                        client->text("{\"temperatures\":"+jsonResponse+"}");
                        //if (this->ws.count() > 0) this->ws.textAll(jsonResponse);
                        if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"temperatures get time\":\""+String(totalTimeMs)+"\"}}\n");
                        onUartQueryComplete = nullptr;
                    };
                    startTimeMs = millis();
                    BeginRetreiveAllTemperatures();
                } else if (command == "states") {
                    onUartQueryComplete = [client,this](String jsonResponse) {
                        unsigned long totalTimeMs = millis()-this->startTimeMs;
                        client->text("{\"states\":"+jsonResponse+"}");
                        //if (this->ws.count() > 0) this->ws.textAll(jsonResponse);
                        if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"states get time\":\""+String(totalTimeMs)+"\"}}\n");
                        onUartQueryComplete = nullptr;
                    };
                    startTimeMs = millis();
                    BeginRetreiveAllStates();
                } else if (command == "lcd") {
                    onUartQueryComplete = [client,this](String jsonResponse) {
                        unsigned long totalTimeMs = millis()-this->startTimeMs;
                        client->text("{\"lcd\":"+convertISO88591toUTF8(jsonResponse)+"}");
                        //if (this->ws.count() > 0) this->ws.textAll(jsonResponse);
                        if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"lcd get time\":\""+String(totalTimeMs)+"\"}}\n");
                        onUartQueryComplete = nullptr;
                    };
                    startTimeMs = millis();
                    BeginRetreiveWholeLCD();
                } else if (command == "writepanel") {
                    if (doc.containsKey("addr") == false) {client->text("{\"error\":\"panel addr parameter missing in json\"}"); return; }
                    if (doc.containsKey("data") == false) {client->text("{\"error\":\"panel data parameter missing in json\"}"); return; }
                    String addrStr = doc["addr"];
                    String dataStr = doc["data"];
                    uint16_t addr = (uint16_t) strtoul(addrStr.c_str(), nullptr, 16);
                    uint16_t data = (uint16_t) strtoul(dataStr.c_str(), nullptr, 16);
                    uartTxBuffer[1] = (uint8_t)Command::WritePanel;
                    lastAction = Action::WebSocketRaw; // for now maybe handle it better
                    Send(addr,data);
                } else if (command == "readpanel") {
                    // TODO fix this
                } else {
                    String errMsg = String("{\"error\":\"unknown cmd: ") + command.c_str() + "\"}";
                    client->text(errMsg);  // Sends a WebSocket text message to the client
                }
            } else {
                client->text("{\"error\":\"cmd parameter missing in json\"}");  // Sends a WebSocket text message to the client
            }
        } else if (info->opcode == WS_BINARY) {
            lastAction = Action::WebSocketRaw;
            UART.write(data, len);
        } else {
            client->text("{\"error\":\"unsupported datatype:"+String(info->opcode)+"\"}");  // Sends a WebSocket text message to the client
        }
    }
}

void REGO600::setup() {
    lastAction = Action::NotSet;
    requestIndex = 0;
    uartTxBuffer[0] = 0x81; // allways 0x81, never to be changed
#if defined(ESP32)
    UART.begin(19200, SERIAL_8N1, 34, 33); // Set correct RX/TX pins for UART
#elif defined(ESP8266)
    UART.begin(19200, SERIAL_8N1);
#endif
    ws.onEvent([this](AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType t, void *a, uint8_t *d, size_t l) {
        this->onWsEvent(s, c, t, a, d, l);
    });

    server.addHandler(&ws);
    
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json; charset=utf-8", "Hello World");
    });
    server.on("/get/*", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String url = request->url();
        // Remove the leading '/'
        if (url.startsWith("/")) url = url.substring(1);
        // Split into parts
        int p1 = url.indexOf('/');
        int p2 = url.indexOf('/', p1 + 1);
        //int p3 = url.indexOf('/', p2 + 1);
        if (p1 == -1 || p2 == -1) request->send(200, "application/json; charset=utf-8", "{\"error\":\"url missing parameters\"}");
        String type = url.substring(p1+1, p2);
        String regname = url.substring(p2 + 1);
        
        type.toLowerCase();
        regname.toUpperCase();
        if (type == "temp") {
            uint32_t addr = 0;
            if (GetTemperatureRegisterAddrFromName(regname.c_str(),&addr) == false) {
                request->send(200, "application/json; charset=utf-8", "{\"error\":\"unknown status regname: "+regname+"\"}");
                return;
            }
            onUartQueryComplete = [this,request](String jsonResponse) {
                unsigned long totalTimeMs = millis()-this->startTimeMs;
                if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"temp get time\":\""+String(totalTimeMs)+"\"}}\n");
                request->send(200, "application/json; charset=utf-8", jsonResponse);
                onUartQueryComplete = nullptr;
            };
            lastAction = Action::ReadTemperature;
            startTimeMs = millis();
            StartSendOneRegisterReadRequest(addr);

        } else if (type == "state") {
            uint32_t addr = 0;
            if (GetStatusRegisterAddrFromName(regname.c_str(),&addr) == false) {
                request->send(200, "application/json; charset=utf-8", "{\"error\":\"unknown status regname: "+regname+"\"}");
                return;
            }
            onUartQueryComplete = [this,request](String jsonResponse) {
                unsigned long totalTimeMs = millis()-this->startTimeMs;
                if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"temp get time\":\""+String(totalTimeMs)+"\"}}\n");
                request->send(200, "application/json; charset=utf-8", jsonResponse);
                onUartQueryComplete = nullptr;
            };
            lastAction = Action::ReadState;
            startTimeMs = millis();
            StartSendOneRegisterReadRequest(addr);
        
        } else {
            request->send(200, "application/json; charset=utf-8", "{\"error\":\"unknown type: "+type+"\"}");
        }
    });
    server.on("/Temperatures", HTTP_GET, [this](AsyncWebServerRequest *request) {
        /*this->pendingRequest = request;
        this->actionDoneDestination = ActionDoneDestination::HttpReq;*/
        
        onUartQueryComplete = [request,this](String jsonResponse) {
            unsigned long totalTimeMs = millis()-this->startTimeMs;
            request->send(200, "application/json; charset=utf-8", jsonResponse);

            if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"temperatures get time\":\""+String(totalTimeMs)+"\"}}\n");
            onUartQueryComplete = nullptr;
        };
        startTimeMs = millis();
        BeginRetreiveAllTemperatures();
    });
    server.on("/States", HTTP_GET, [this](AsyncWebServerRequest *request) {
        /*this->pendingRequest = request;
        this->actionDoneDestination = ActionDoneDestination::HttpReq;*/
        onUartQueryComplete = [this,request](String jsonResponse) {
            unsigned long totalTimeMs = millis()-this->startTimeMs;
            request->send(200, "application/json; charset=utf-8", jsonResponse);
            if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"states get time\":\""+String(totalTimeMs)+"\"}}\n");
            onUartQueryComplete = nullptr;
        };
        startTimeMs = millis();
        BeginRetreiveAllStates();
    });
    server.on("/LCD", HTTP_GET, [this](AsyncWebServerRequest *request) {
        /*this->pendingRequest = request;
        this->actionDoneDestination = ActionDoneDestination::HttpReq;*/
        onUartQueryComplete = [this,request](String jsonResponse) {
            unsigned long totalTimeMs = millis()-this->startTimeMs;
            request->send(200, "application/json; charset=utf-8", jsonResponse);
            if (this->ws.count() > 0) this->ws.textAll("{\"debug\":{\"lcd get time\":\""+String(totalTimeMs)+"\"}}\n");
            //request->send(200, "application/json; charset=windows-1252", jsonResponse);
            onUartQueryComplete = nullptr;
        };
        startTimeMs = millis();
        BeginRetreiveWholeLCD();
    });
    server.begin();
}

void REGO600::SendNextRequest() {
    SendReq(requests[requestIndex++].address);
}

void REGO600::SendReq(uint16_t address) {
    uartRxBufferIndex = 0;
    SetRequestAddr(address);
    CalcAndSetTxChecksum();
    UART.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}

void REGO600::Send(uint16_t address, uint16_t data) {
    uartRxBufferIndex = 0;
    SetRequestAddr(address);
    SetRequestData(data);
    CalcAndSetTxChecksum();
    UART.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}

void REGO600::StartSendOneRegisterReadRequest(uint16_t address) {
    currentExpectedRxLength = (uint8_t)CommandRxDataLenght::ReadRegister;
    uartTxBuffer[1] = (uint8_t)Command::ReadRegister;
    SendReq(address);
}

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

void REGO600::task_loop() {
    if (lastAction == Action::NotSet) {
        while (UART.available()) { // to make sure that any garbage gets collected
            UART.read();
        }
        return;
    }
    if (lastAction == Action::WebSocketRaw) {
        // Read all available UART data
        while (UART.available()) {
            if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartRxBufferIndex++] = UART.read();
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
        while (UART.available()) {
            if (uartRxBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartRxBufferIndex++] = UART.read();
                if (uartRxBufferIndex == currentExpectedRxLength) {
                    // RX is done
                    if (lastAction == Action::ReadWholeLCD) {
                        RequestsWholeLCD_Task();
                    } else if (lastAction == Action::ReadTemperatures) {
                        RequestsAllTemperatures_Task();
                    } else if (lastAction == Action::ReadStates) {
                        RequestsAllStates_Task();
                    } else if (lastAction == Action::ReadState) {
                        RequestOneState_Task();
                    } else if (lastAction == Action::ReadTemperature) {
                        RequestOneTemperature_Task();
                    } else {
                        if (this->ws.count() > 0) this->ws.textAll("{\"error\":\"last action not set\"}\n");
                    }
                }
            } else {
                UART.read(); // must read remaining to get out of loop
                if (this->ws.count() > 0) this->ws.textAll("{\"error\":\"uartRxBuffer full\"}\n");
            }
        }
    }
}