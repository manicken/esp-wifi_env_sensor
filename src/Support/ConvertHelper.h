#pragma once

#include <Arduino.h>

#if defined(ESP8266)
#define DEBUG_UART Serial1
#elif defined(ESP32)
#define DEBUG_UART Serial
#endif
#include <cstring>
#include "Logger.h"

namespace Convert
{
    std::string floatToString(float value);
    bool isInteger(const char* str);

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize);
    bool HexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize);

    bool contains(int *array, size_t arraySize, uint8_t value);

    uint64_t reverseMACaddress(uint64_t addr);

    /** with padding */
    String toHex(const char *data, size_t len);
    String convertISO88591toUTF8(const String &input);
    bool IsValidHexString(const char* str);

    std::string toHex(uint8_t value);
    std::string toHex(uint32_t value);
    std::string toHex(uint64_t value);
    std::string toBin(uint8_t value);

} // namespace Convert