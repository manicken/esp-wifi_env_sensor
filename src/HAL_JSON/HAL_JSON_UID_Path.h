
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <Arduino.h> // Needed for String class
#include "../Support/CharArrayHelpers.h"
namespace HAL_JSON {

    // Encode UID from ASCII string (up to 8 chars)
    uint64_t encodeUID(const char* str);
    uint64_t encodeUID(const char* str, uint32_t count);
    std::string decodeUID(uint64_t uid);

    struct UIDPath {
        static constexpr uint64_t UID_INVALID = 1;
        uint64_t* items = nullptr;
        uint32_t itemCount = 0;

        //uint64_t group = 0;
        //uint64_t item  = 0;

        UIDPath();
        UIDPath(const char* uidStr);
        UIDPath(const std::string& uidStr);

        uint64_t root() const;
        uint64_t last() const;
        uint64_t byIndex(uint32_t index) const;
/*don't think i need theese operations
        bool operator==(const UIDPath& other) const;
        bool operator==(const char* uidStr) const;
        bool operator==(const std::string& uidStr) const;
        */
    };

}