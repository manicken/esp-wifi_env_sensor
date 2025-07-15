
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#ifndef _WIN32
#include <Arduino.h> // Needed for String class
#else
#define F(x) x
#endif
#include "../Support/CharArrayHelpers.h"
#include "../Support/ConvertHelper.h"
#include "../Support/Logger.h"

namespace HAL_JSON {

    struct HAL_UID {
        union
        {
            char str[8];
            uint64_t val;
        };
        
    };

    // Encode UID from ASCII string (up to 8 chars)
    uint64_t encodeUID(const char* str);
    uint64_t encodeUID(const char* str, uint32_t count);
    std::string decodeUID(uint64_t uid);

    class UIDPath {
    private:
        uint64_t* items = nullptr;
        //HAL_UID* items = nullptr; // future implementation so that encodeUID and decodeUID won't be needed
        uint32_t itemCount = 0;
        uint32_t currentItemIndex = 0;

    public:
        enum class ToStringType {
            String,
            Raw
        };
        static constexpr uint64_t UID_INVALID = 1;

        UIDPath();
        ~UIDPath();
        UIDPath(const char* uidStr);
        UIDPath(const std::string& uidStr);
        UIDPath(const UIDPath& other) = delete; // Copy constructor
        UIDPath& operator=(const UIDPath& other) = delete; // Copy assignment
        UIDPath(UIDPath&& other) = delete; // Move constructor
        UIDPath& operator=(UIDPath&& other) = delete; // Move assignment
        /** Ideally, the program should be designed to avoid needing this function. 
         * but the only place where it should be used is at start of Manager::findDevice 
         * that way we never need to check it while using other functions
         */
        bool empty() const;
        
        uint32_t count();
        uint64_t getCurrentUID();
        uint64_t resetAndGetFirst();
        uint64_t getNextUID();
        uint64_t peekNextUID();
        bool isLast();

        std::string ToString(ToStringType type = ToStringType::String);

        //uint64_t byIndex(uint32_t index); // usage of this function don't make any sense
    };

}