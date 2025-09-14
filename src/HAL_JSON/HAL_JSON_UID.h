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
        static constexpr uint64_t UID_NOT_SET = 0;
        union
        {
            char str[Size];
            uint64_t val;
            //uint32_t values[Size/4];
        };
        
        HAL_UID();
        HAL_UID(uint64_t v);
        /** basically: val == HAL_UID::UID_NOT_SET */
        bool NotSet();
        /** basically: (val != HAL_UID::UID_NOT_SET) && (val != HAL_UID::UID_INVALID) */
        bool IsSet();
        /** basically: val == HAL_UID::UID_INVALID */
        bool Invalid();
        HAL_UID& operator=(const HAL_UID& v);

    };
    bool operator==(const HAL_UID& lhs, const HAL_UID& rhs);
    bool operator!=(const HAL_UID& lhs, const HAL_UID& rhs);

    // Encode UID from ASCII string (up to 8 chars)
    HAL_UID encodeUID(const char* str);
    HAL_UID encodeUID(const ZeroCopyString& zcStr);
    HAL_UID encodeUID(const char* str, uint32_t count);
    std::string decodeUID(HAL_UID& uid);
}