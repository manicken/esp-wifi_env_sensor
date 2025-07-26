
#include "HAL_JSON_UID.h"

namespace HAL_JSON {
    HAL_UID::HAL_UID():val(0) {};
    HAL_UID::HAL_UID(uint64_t v):val(v) {};

    HAL_UID& HAL_UID::operator=(const HAL_UID& v) {
        this->val = v.val;
        return *this;
    }
    bool HAL_UID::NotSet() { return val == 0; }
    bool HAL_UID::IsSet() { return val != 0; }
    bool HAL_UID::Invalid() {return val == HAL_UID::UID_INVALID; }

    bool operator==(const HAL_UID& lhs, const HAL_UID& rhs) {
        return lhs.val == rhs.val;
    }
    bool operator==(const HAL_UID& lhs, const uint64_t& rhs) {
        return lhs.val == rhs;
    }
    bool operator==(const uint64_t& lhs, const HAL_UID& rhs) {
        return lhs == rhs.val;
    }

    bool operator!=(const HAL_UID& lhs, const HAL_UID& rhs) {
        return lhs.val != rhs.val;
    }

    HAL_UID encodeUID(const CharArray::ZeroCopyString& zcStr){
        HAL_UID out;
        if (zcStr.Length() == 0) return out;
        
        const char* p = zcStr.start;
        const char* const end = zcStr.end;
        int i = 0;
        while (p < end && i < HAL_UID::Size) {
            out.str[i] = *p;//  ((HAL_UID)(uint8_t)*p) << (8 * (7 - i++)); // big-endian
        }
        return out;
    }

    HAL_UID encodeUID(const char* str) {
        HAL_UID out;
        if (!str || (strlen(str) == 0)) return out;
        for (int i = 0; i < HAL_UID::Size && str[i]; ++i) {
            out.str[i] = str[i];//out |= ((uint64_t)(uint8_t)str[i]) << (8 * (7 - i)); // big-endian
        }
        return out;
    }

    HAL_UID encodeUID(const char* str, uint32_t count) {
        HAL_UID out;
        if (!str || (strlen(str) == 0) || (count == 0)) return out;
        for (uint32_t i = 0; i < HAL_UID::Size && i < count && str[i]; ++i) {
            out.str[i] = str[i];//out |= ((uint64_t)(uint8_t)str[i]) << (8 * (7 - i)); // big-endian
        }
        return out;
    }

    std::string decodeUID(HAL_UID uid) {
        if (uid == 0) return "<zero>";
        if (uid.Invalid()) return "<invalid>";
        char str[9] = {}; // 8 chars + null terminator
        for (int i = 0; i < HAL_UID::Size; ++i) {
            // Extract each byte from most significant to least significant
            str[i] = uid.str[i];// static_cast<char>((uid >> (8 * (7 - i))) & 0xFF);
            if (str[i] == '\0') { // Stop if null terminator found
                str[i] = '\0';
                break;
            }
        }
        return std::string(str);
    }
}