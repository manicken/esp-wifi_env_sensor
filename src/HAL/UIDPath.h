
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <Arduino.h> // Needed for String class
namespace HAL {

    // Encode UID from ASCII string (up to 8 chars)
    uint64_t encodeUID(const char* str);
    std::string decodeUID(uint64_t uid);

    struct UIDPath {
        uint64_t group = 0;
        uint64_t item  = 0;

        UIDPath();
        UIDPath(const char* itemStr, const char* groupStr = nullptr);
        UIDPath(std::string itemStr, std::string groupStr = "");

        bool isItemOnly() const;

        bool operator==(const UIDPath& other) const;
        bool operator==(const char* uidStr) const;
        bool operator==(const std::string& uidStr) const;
    };

}