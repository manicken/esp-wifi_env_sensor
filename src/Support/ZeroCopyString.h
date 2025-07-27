#pragma once

#ifndef _WIN32
#include <Arduino.h>
#endif
#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

namespace HAL_JSON {
    /**
     * Note: 'end' is exclusive, i.e., points just past the last valid character
     */
    struct ZeroCopyString
    {
        const char* start;
        /** Note: 'end' is exclusive, i.e., points just past the last valid character */
        const char* end;
        /** 
         * Returns the lenght of the string,
         * also returns length of zero when either start or end is nullptr
         */
        size_t Length() const;
        /** 
         * Returns a std::string copy of the string, 
         * note this is mostly intended for debug or when the string is needed somewhere else 
         */
        std::string ToString() const;
        ZeroCopyString();
        /** Returns pointer to first occurrence of ch, or nullptr */
        const char* FindChar(char ch) const;
        /** Counts how many times ch appears in the range */
        uint32_t CountChar(char ch) const;
        /** Returns dynamically allocated array of indices (caller must delete[]) */
        const uint32_t* GetIndicies(char ch, uint32_t& outCount) const;
        /** Returns dynamically allocated array of pointers (caller must delete[]) */
        const char** GetPointers(char ch, uint32_t& outCount) const;
    };
    
}