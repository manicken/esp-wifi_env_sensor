#include <Arduino.h>
#include <cctype>
#include <cstring>

namespace CharArray {
    uint32_t countChar(const char* str, char ch);
    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount);
    /** case insensitive compare */
    bool equalsIgnoreCase(const char* a, const char* b);
}