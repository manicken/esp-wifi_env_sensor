#include <Arduino.h>
#include <cctype>
#include <cstring>

namespace CharArray {
    uint32_t countChar(const char* str, char ch);
    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount);
    const char** getPointers(const char* str, char ch, uint32_t& outCount);
    /** case insensitive compare */
    bool equalsIgnoreCase(const char* a, const char* b);

    struct StringView {
        const char* ptr;
        size_t len;

        StringView() : ptr(nullptr), len(0) {}
        StringView(const char* p, size_t l) : ptr(p), len(l) {}

        bool empty() const { return len == 0; }

        std::string toString() const { return std::string(ptr, len); } // for debug/logging
    };

}