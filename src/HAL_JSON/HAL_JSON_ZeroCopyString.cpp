
#include "HAL_JSON_ZeroCopyString.h"

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
    const char* ZeroCopyString::FindString(const char* str, const char* _start) const {
        if (!str || !start || !end || (end <= start)) return nullptr;
        
        // Validate _start
        const char* searchStart = (_start && _start >= start && _start < end) ? _start : start;
        size_t remaining = end - searchStart;

        size_t needleLen = std::strlen(str);
        if (needleLen == 0 || needleLen > remaining) return nullptr;

        for (const char* p = searchStart; p <= end - needleLen; ++p) {
            if (std::strncmp(p, str, needleLen) == 0) {
                return p;
            }
        }
        return nullptr;
    }
    const char* ZeroCopyString::FindAnyString(const char* const* candidates, const char* _start, const char** matchedCandidate) const {
        if (matchedCandidate) *matchedCandidate = nullptr;
        const char* earliestMatch = nullptr;
        const char* earliestKeyword = nullptr;
  
        for (int i = 0; candidates[i] != nullptr; ++i) {
            const char* found = FindString(candidates[i], _start);
            if (found && (!earliestMatch || found < earliestMatch)) {
                earliestMatch = found;
                earliestKeyword = candidates[i];
            }
        }

        if (matchedCandidate) *matchedCandidate = earliestKeyword;
        return earliestMatch;
    }

    const char* ZeroCopyString::FindFirstMatchingString(const char* const* candidates, const char* _start, const char** matchedCandidate) const {
        if (!start || !end || (end <= start)) return nullptr;
        if (matchedCandidate) *matchedCandidate = nullptr;

        const char* searchStart = (_start && _start >= start && _start < end) ? _start : start;
        size_t maxSearchLen = end - searchStart;

        for (size_t offset = 0; offset < maxSearchLen; ++offset) {
            const char* pos = searchStart + offset;
            for (int i = 0; candidates[i] != nullptr; ++i) {
                size_t candidateLen = std::strlen(candidates[i]);
                if ((pos + candidateLen) <= end && std::strncmp(pos, candidates[i], candidateLen) == 0) {
                    if (matchedCandidate) *matchedCandidate = candidates[i];
                    return pos;
                }
            }
        }

        return nullptr;
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

    uint32_t ZeroCopyString::CountString(const char* str) const {
        if (!str || !start || !end || (end-start) <= 0) return 0;
        size_t length = (end-start); // safe to do this here

        size_t needleLen = std::strlen(str);
        if (needleLen == 0 || needleLen > length) return 0;

        uint32_t count = 0;
        for (size_t i = 0; i <= length - needleLen;) {
            if (std::strncmp(&start[i], str, needleLen) == 0) {
                ++count;
                i += needleLen; // move past current match
            } else {
                ++i;
            }
        }
        return count;
    }

    uint32_t ZeroCopyString::CountAnyString(const char* const* candidates) const {
        uint32_t count = 0;
        for (int i = 0; candidates[i] != nullptr; ++i) {
            count += CountString(candidates[i]);
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

    ZeroCopyString ZeroCopyString::SplitOffHead(char delimiter) {
        if (!start || start >= end) return ZeroCopyString(nullptr, nullptr);
        
        const char* splitPos = FindChar(delimiter);
        const char* newStartPos = start;
        if (splitPos == nullptr) {
            start = end;
            return ZeroCopyString(newStartPos, end);
        }
        start = splitPos + 1;
        return ZeroCopyString(newStartPos, splitPos);
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
    bool ZeroCopyString::EqualsIC(const ZeroCopyString& other) const {
        int thisLen = Length();
        if (thisLen == 0) return false;
        int otherLen = other.Length();
        if (otherLen == 0) return false;
        if (thisLen != otherLen) return false;
        const char* a = start;
        const char* endA = end;
        const char* b = other.start;
        const char* endB = other.end;
        while (a < endA && b < endB) {
            //const char aLC = (*a >= 'A' && *a < 'Z')?(*a+' '):*a;
            //const char bLC = (*b >= 'A' && *b < 'Z')?(*b+' '):*b;
            //if (aLC != bLC) return false;
            if (std::tolower(*a) != std::tolower(*b)) {
                return false;
            }
            a++;
            b++;
        }
        return *a == *b; // Ensure both strings ended
    }
    bool ZeroCopyString::EqualsIC(const char* cstr) const {
        if (!cstr) return false;
        int thisLen = Length();
        if (thisLen == 0) return false;
        int cstrLen = strlen(cstr);
        if (cstrLen == 0) return false;
        if (thisLen != cstrLen) return false;
        const char* a = start;
        const char* endA = end;

        while (a < endA && *cstr != '\0') {
            //const char aLC = (*a >= 'A' && *a < 'Z')?(*a+' '):*a;
            //const char bLC = (*cstr >= 'A' && *cstr < 'Z')?(*cstr+' '):*cstr;
            //if (aLC != bLC) return false;
            if (std::tolower(*a) != std::tolower(*cstr)) {
                return false;
            }
            a++;
            cstr++;
        }
        return *a == *cstr; // Ensure both strings ended
    }
    bool ZeroCopyString::EqualsICAny(const char* const* candidates) const {
        for (int i = 0; candidates[i] != nullptr; ++i) {
            if (EqualsIC(candidates[i])) {
                return true;
            }
        }
        return false;
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
    bool ZeroCopyString::ConvertTo_int32(int32_t& outValue) const {
        uint32_t tempVal = 0;
        if (ConvertTo_uint32(tempVal) == false) return false;
        const char* p = start;
        // Skip leading spaces, and as ConvertTo_uint32 allready verified that the string is correct so far no failsafe checks are needed
        while (*p == ' ') p++;
        bool negative = (*p == '-');
        // Clamp range to avoid overflow
        if (!negative && tempVal > INT32_MAX) return false;
        if (negative && tempVal > static_cast<uint32_t>(INT32_MAX) + 1) return false;

        outValue = negative ? -static_cast<int32_t>(tempVal) : static_cast<int32_t>(tempVal);
        return true;
    }

    bool ZeroCopyString::ConvertTo_float(float& outValue) const {
        if (Length() == 0) return false;

        const char* p = start;
        const char* const _end = end;

        // Skip leading spaces
        while (p < _end && *p == ' ') p++;

        // Handle optional sign
        bool negative = false;
        if (p < _end && (*p == '+' || *p == '-')) {
            negative = (*p == '-');
            p++;
        }

        float value = 0.0;
        bool decimalSeen = false;
        float decimalFactor = 0.1;
        bool digitSeen = false;

        while (p < _end) {
            char ch = *p;

            if (ch >= '0' && ch <= '9') {
                digitSeen = true;
                if (!decimalSeen) {
                    value = value * 10.0 + (ch - '0');
                } else {
                    value += (ch - '0') * decimalFactor;
                    decimalFactor *= 0.1;
                }
            } else if ((ch == '.' || ch == ',') && !decimalSeen) {
                // First decimal point (treat both '.' and ',' as decimal separator here)
                decimalSeen = true;
            } else if (ch == ',' || ch == ' ') {
                // Skip commas as thousands separators or space padding
            } else {
                break;  // Invalid character or end of number
            }

            p++;
        }

        if (!digitSeen) return false;

        outValue = negative ? -value : value;
        return true;
    }

    bool operator==(const ZeroCopyString& lhs, const ZeroCopyString& rhs) { return lhs.Equals(rhs); }
    bool operator!=(const ZeroCopyString& lhs, const ZeroCopyString& rhs) { return !lhs.Equals(rhs); }
    bool operator==(const char* lhs, const ZeroCopyString& rhs) { return rhs.Equals(lhs); }
    bool operator!=(const char* lhs, const ZeroCopyString& rhs) { return !rhs.Equals(lhs); }
    bool operator==(const ZeroCopyString& lhs, const char* rhs) { return lhs.Equals(rhs); }
    bool operator!=(const ZeroCopyString& lhs, const char* rhs) { return !lhs.Equals(rhs); }
}