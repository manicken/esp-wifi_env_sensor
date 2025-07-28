
#include "ZeroCopyString.h"

namespace HAL_JSON {
    ZeroCopyString::ZeroCopyString() : start(nullptr), end(nullptr) { } // empty 'string'
    ZeroCopyString::ZeroCopyString(const char* start, const char* end) : start(start), end(end) { 
        if (this->end == nullptr) this->start = nullptr; 
        else if (this->start == nullptr) this->end = nullptr;
    }
    ZeroCopyString::ZeroCopyString(const char* cstr) : start(cstr), end(cstr ? cstr + strlen(cstr) : cstr) {
        if (this->end == nullptr) this->start = nullptr; 
        else if (this->start == nullptr) this->end = nullptr;
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
    const char* ZeroCopyString::FindChar(char ch, const char* _start) const {
        // Make sure _start is inside [start, end)
        if (( _start == nullptr ) || ( _start < start ) || ( _start >= end )) return nullptr;

        // Number of bytes left to search
        size_t remaining = end - _start;

        return static_cast<const char*>(memchr(_start, ch, remaining));
    }
    ZeroCopyString ZeroCopyString::substring(const char* sub_start, const char* sub_end) const {
        if (!sub_start || !sub_end) {
            // Return empty string or handle error
            return ZeroCopyString{nullptr, nullptr};  // empty
        }
        // Validate input pointers: must be within [start, end]
        if (sub_start < start) sub_start = start;
        if (sub_end > end) sub_end = end;
        if (sub_start > sub_end) sub_start = sub_end;  // empty string if reversed

        return ZeroCopyString{sub_start, sub_end};
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

    bool ZeroCopyString::Equals(const ZeroCopyString& other) const {
        size_t len1 = Length();
        size_t len2 = other.Length();

        if (len1 != len2)
            return false;

        // Compare memory block directly
        return std::memcmp(start, other.start, len1) == 0;
    }
    bool ZeroCopyString::Equals(const char* cstr) const {
        if (!cstr)
            return false;

        size_t len = Length();
        return std::strlen(cstr) == len && std::memcmp(start, cstr, len) == 0;
    }

    bool ZeroCopyString::ValidNumber() const {
        if (Length() == 0) return false;

        // Quick count checks
        if (CountChar('.') > 1) return false;
        if (CountChar(',') > 1) return false;
        if (CountChar('-') > 1) return false;
        if (CountChar('+') > 1) return false;

        const char* p = start;
        const char* const _end = end;

        // Skip leading spaces
        while (p < _end && *p == ' ') p++;

        // Optional sign at the start
        if (p < _end && (*p == '-' || *p == '+')) p++;

        if (p == _end) return false; // Only sign and spaces?

        bool decimalSeen = false;
        bool commaSeen = false;

        for (; p < _end; ++p) {
            char c = *p;
            if (isdigit(c)) {
                continue;
            } else if (c == '.') {
                if (decimalSeen || commaSeen) return false;
                decimalSeen = true;
            } else if (c == ',') {
                if (decimalSeen || commaSeen) return false;
                commaSeen = true;
            } else if (c == ' ') {
                // spaces inside number: either disallow or allow only trailing spaces
                // Here, disallow spaces inside number:
                return false;
            } else {
                return false;
            }
        }
        return true;
    }

    bool ZeroCopyString::ConvertTo_uint32(uint32_t& outValue) const {
        if (Length() == 0) return false;

        const char* p = start;
        const char* const _end = end;

        // Skip leading spaces
        while (p < _end && *p == ' ') p++;

        // Skip optional '+' or '-' (but remember, we're returning unsigned!)
        if (p < _end && (*p == '+' || *p == '-')) p++;

        uint32_t value = 0;

        while (p < _end) {
            char ch = *p;
            if (ch >= '0' && ch <= '9') {
                value = value * 10 + (ch - '0');
            } else if (ch == ',') {
                // allow comma as thousands separator comment the following
                break;
            } else if (ch == '.') {
                // Allways Stop at decimal separator
                break;
            } else if (ch == ' ') {
                // Allow spaces inside? optional.
                // Skip or break depending on policy, comment the following if should be allowed
                break;
            } else {
                return false; // Invalid character
            }
            p++;
        }

        outValue = value;
        return true;
    }


}