
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

