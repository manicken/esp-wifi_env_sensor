
#include "REGO600.h"

const REGO600::Request RequestsWholeLCD[] = {
    {0x0000, 0x0000},
    {0x0001, 0x0001},
    {0x0002, 0x0002},
    {0x0003, 0x0003}
};
const size_t RequestsWholeLCD_Count = 4;
const REGO600::Request RequestsAllTemperatures[] = {
    {0x0209, 0x0000},
    {0x020A, 0x0000},
    {0x020B, 0x0000},
    {0x020D, 0x0000},
    {0x020E, 0x0000},
    {0x020F, 0x0000},
    {0x0210, 0x0000},
    {0x0211, 0x0000},
    {0x0212, 0x0000},
};
const size_t RequestsAllTemperatures_Count = 9;

REGO600::REGO600()
    : server(REGO600_WS_PORT), ws("/rego600ws"), UART2(Serial2) // Init UART2 as HardwareSerial(2)
{}

void REGO600::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;

        if (info->opcode == WS_BINARY) {
            lastCommand = Command::None;
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

}

void REGO600::BeginRetreiveWholeLCD() {
    lastCommand = Command::ReadLcd;
    currentExpectedRxLength = (uint8_t)CommandRxDataLenght::ReadLcd;
    requestCount = RequestsWholeLCD_Count;
    requestIndex = 0;
    requests = RequestsWholeLCD;
    uartTxBuffer[1] = (uint8_t)lastCommand;
    SetRequest(requests[requestIndex++]);
    CalcAndSetTxChecksum();
    UART2.write(uartTxBuffer, REGO600_UART_TX_BUFFER_SIZE);
}

void REGO600::SetRequest(REGO600::Request req) {
    uartTxBuffer[2] = (req.address >> 14) & 0x7F;
    uartTxBuffer[3] = (req.address >> 7) & 0x7F;
    uartTxBuffer[4] = req.address & 0x7F;
    uartTxBuffer[5] = (req.data >> 14) & 0x7F;
    uartTxBuffer[6] = (req.data >> 7) & 0x7F;
    uartTxBuffer[7] = req.data & 0x7F;
}

void REGO600::CalcAndSetTxChecksum() {
    uint8_t chksum = 0;
    for (int i=REGO600_UART_TX_CHKSUM_START_INDEX;i<REGO600_UART_TX_CHKSUM_END_INDEX;i++) {
        chksum ^= uartTxBuffer[i];
    }
    uartTxBuffer[REGO600_UART_TX_CHKSUM_END_INDEX] = chksum;
}

void REGO600::task_loop() {
    if (lastCommand == Command::None) {
        // Read all available UART data
        while (UART2.available()) {
            if (uartBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartBufferIndex++] = UART2.read();
                lastByteTime = millis(); // update last byte time
            }
        }

        // If buffer is not empty and timeout passed, send it
        if (uartBufferIndex > 0 && (millis() - lastByteTime > FLUSH_TIMEOUT_MS)) {
            ws.binaryAll(uartRxBuffer, uartBufferIndex);
            uartBufferIndex = 0;
        }
    }
    else {
        while (UART2.available()) {
            if (uartBufferIndex < REGO600_UART_RX_BUFFER_SIZE) {
                uartRxBuffer[uartBufferIndex++] = UART2.read();
                if (uartBufferIndex == currentExpectedRxLength) {
                    // RX is done
                    if (lastCommand == Command::ReadLcd) {

                    }
                }
            }
        }
    }
}