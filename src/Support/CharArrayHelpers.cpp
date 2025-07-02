#include "CharArrayHelpers.h"

namespace CharArray {
    uint32_t countChar(const char* str, char ch) {
        uint32_t count = 0;
        while (*str) {
            if (*str == ch) count++;
            str++;
        }
        return count;
    }
    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount) {
        outCount = countChar(str, ch);
        if (outCount == 0) return nullptr;
        uint32_t* indicies = new uint32_t[outCount];
        uint32_t index = 0;
        uint32_t strIndex = 0;
        while (*str) {
            if (*str == ch) { 
                indicies[index++] = strIndex;
            }
            str++;
            strIndex++;
        }
        return indicies;
    }

    

    bool equalsIgnoreCase(const char* a, const char* b) {
        while (*a && *b) {
            if (std::tolower(*a) != std::tolower(*b)) {
                return false;
            }
            ++a;
            ++b;
        }
        return *a == *b; // Ensure both strings ended
    }
}