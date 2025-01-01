#pragma once

#include <Arduino.h>

#if defined(ESP8266)
#define DEBUG_UART Serial1
#elif defined(ESP32)
#define DEBUG_UART Serial
#endif

namespace Convert
{
    std::string floatToString(float value);
    bool isInteger(const char* str);

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize);
    bool convertHexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize);

    bool contains(int *array, size_t arraySize, uint8_t value);

} // namespace Convert