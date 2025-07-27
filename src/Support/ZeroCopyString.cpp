
#include "ZeroCopyString.h"

namespace HAL_JSON {
    ZeroCopyString::ZeroCopyString() {
        start = nullptr;
        end = nullptr;
    }
    size_t ZeroCopyString::Length() const {
        if (start == nullptr || end == nullptr || end < start ) return 0;
        return end-start;
    }
    std::string ZeroCopyString::ToString() const {
        if (!start || !end || end < start) return {};
        return std::string(start, end);
    }
    const char* ZeroCopyString::FindChar(char ch) const {
        return static_cast<const char*>(memchr(start, ch, Length()));
    }

    uint32_t ZeroCopyString::CountChar(char ch) const {
        if (Length() == 0) return 0;
        uint32_t count = 0;
        const char* p = start;
        const char* const _end = end;
        while (p < _end) {
            if (*p == ch) count++;
            p++;
        }
        return count;
    }
    const uint32_t* ZeroCopyString::GetIndicies(char ch, uint32_t& outCount) const {
        if (Length() == 0) return nullptr;
        outCount = CountChar(ch);
        if (outCount == 0) return nullptr;
        uint32_t* indicies = new uint32_t[outCount];
        uint32_t index = 0;
        uint32_t strIndex = 0;
        const char* p = start;
        const char* const _end = end;
        while (p < _end) {
            if (*p == ch) { 
                indicies[index++] = strIndex;
            }
            p++;
            strIndex++;
        }
        return indicies;
    }
    const char** ZeroCopyString::GetPointers(char ch, uint32_t& outCount) const {
        if (Length() == 0) return nullptr;
        outCount = CountChar(ch);
        if (outCount == 0) return nullptr;
        const char** pointers = new const char*[outCount];
        uint32_t index = 0;
        const char* p = start;
        const char* const _end = end;
        while (p < _end) {
            if (*p == ch) { 
                pointers[index++] = p;
            }
            p++;
        }
        return pointers;
    }
}