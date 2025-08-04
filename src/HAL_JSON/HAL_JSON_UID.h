#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include <Arduino.h> // Needed for String class

#include "../Support/CharArrayHelpers.h"
#include "HAL_JSON_ZeroCopyString.h"
#include "../Support/ConvertHelper.h"
#include "../Support/Logger.h"

namespace HAL_JSON {
    struct HAL_UID {
        static constexpr size_t Size = 8;
        static constexpr uint64_t UID_INVALID = 1;
        union
        {
            char str[Size];
            uint64_t val;
        };
        
        HAL_UID();
        HAL_UID(uint64_t v);
        bool NotSet();
        bool IsSet();
        /** invalid is a special state, not to be confused with not set */
        bool Invalid();
        HAL_UID& operator=(const HAL_UID& v);

    };
    //bool operator==(const HAL_UID& lhs, const uint64_t& rhs);
    //bool operator==(const uint64_t& lhs, const HAL_UID& rhs);
    bool operator==(const HAL_UID& lhs, const HAL_UID& rhs);
    bool operator!=(const HAL_UID& lhs, const HAL_UID& rhs);

    // Encode UID from ASCII string (up to 8 chars)
    HAL_UID encodeUID(const char* str);
    HAL_UID encodeUID(const ZeroCopyString& zcStr);
    HAL_UID encodeUID(const char* str, uint32_t count);
    std::string decodeUID(HAL_UID uid);
}