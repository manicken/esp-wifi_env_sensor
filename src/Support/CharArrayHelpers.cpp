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

    const char** getPointers(const char* str, char ch, uint32_t& outCount) {
        outCount = countChar(str, ch);
        if (outCount == 0) return nullptr;
        const char** indicies = new const char*[outCount];
        uint32_t index = 0;
        while (*str) {
            if (*str == ch) { 
                indicies[index++] = str;
            }
            str++;
            //strIndex++;
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

    StringView::StringView() : ptr(nullptr), len(0) {}
    StringView::StringView(const char* p, size_t l) : ptr(p), len(l) {}
    bool StringView::empty() const { return len == 0; }
    std::string StringView::toString() const { return std::string(ptr, len); } // for debug/logging
}