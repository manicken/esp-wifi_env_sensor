
// this file is for testing Rule Engine on PC environment


//#ifdef _WIN32

#include <iostream>
#include <string>
#include <cstdint>
#include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"

std::string toBin(uint8_t value) {
        char bitStr[9];
        uint32_t divider = 0x80;
       // String bin = String(value, BIN);  // Convert to binary
        //while (bin.length() < 8) {
       //     bin = "0" + bin;
        //}
        for (int i=0;i<8;i++) {
            bitStr[i] = ((value / divider) & 0x01) + '0';
            value %= divider;
            divider >>=1;
        }
        bitStr[8] = 0;
        return std::string(bitStr);
    }

std::string toHex(uint32_t value) {
        char hexStr[9];
        uint32_t divider = 0x10000000;
        for (int i=0;i<8;i++) {
            uint32_t nibble = (value/divider)&0xF;
            hexStr[i] = (nibble >= 10) ? (nibble + ('A'-10)) : (nibble + '0');
            value %=divider;
            divider>>=4;
        }
        hexStr[8] = 0x00;
        return std::string(hexStr);
    }
    std::string toHex(uint64_t value) {
        char hexStr[17];
        uint64_t divider = 0x1000000000000000;
        for (int i=0;i<16;i++) {
            uint32_t nibble = (value/divider)&0xF;
            hexStr[i] = (nibble >= 10) ? (nibble + ('A'-10)) : (nibble + '0');
            value %=divider;
            divider>>=4;
        }
        hexStr[16] = 0x00;
        return std::string(hexStr);
    }

    int main() {
        HAL_JSON::RuleEngine::validateRuleSet("on relay#toggle do if relay#state == 1 do led = 1 endif endon");
        std::cout << "Running on Windows (MinGW)" << std::endl;
        std::cout << toHex((uint64_t)0x123456789ABCDEF0) << std::endl;
        std::cout << toHex((uint32_t)0x12345678) << std::endl;
        std::cout << toBin((uint32_t)0xAA) << std::endl;
        std::cout << toBin((uint32_t)0x55) << std::endl;
        std::cout << toBin((uint32_t)0x18) << std::endl;
        std::cout << toBin((uint32_t)0x81) << std::endl;
    }

//#endif