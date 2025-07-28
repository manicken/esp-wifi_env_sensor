#pragma once

#include <Arduino.h>

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
        ZeroCopyString(const char* start, const char* end);
        /**
         * Create a ZeroCopyString from a null-terminated C string.
         * 
         * Note: The original string data must remain valid and unchanged
         * for the entire lifetime of the ZeroCopyString instance,
         * as this class does not copy or own the data.
         */
        ZeroCopyString(const char* cstr);
        /** Returns pointer to first occurrence of ch, or nullptr */
        const char* FindChar(char ch) const;
        /** Returns pointer to first occurrence of ch, or nullptr from the start pointer given*/
        const char* FindChar(char ch, const char* start) const;
        /** Returns a new ZeroCopyString from the given pointers */
        ZeroCopyString substring(const char* sub_start, const char* sub_end) const;
        /** Counts how many times ch appears in the range */
        uint32_t CountChar(char ch) const;
        /** Returns dynamically allocated array of indices (caller must delete[]) */
        const uint32_t* GetIndicies(char ch, uint32_t& outCount) const;
        /** Returns dynamically allocated array of pointers (caller must delete[]) */
        const char** GetPointers(char ch, uint32_t& outCount) const;

        /** 
         * Splits off the leading segment of the string up to the given delimiter. 
         * 
         * Modifies the current ZeroCopyString in-place so that it now starts 
         * just after the delimiter. The extracted head segment is returned 
         * as a new ZeroCopyString.
         * 
         * So the original string need to be copied if one need that later
         * 
         * Example: 
         *   ZeroCopyString z("foo/bar/baz");
         *   ZeroCopyString head = z.SplitOffHead('/');
         *   // head = "foo", z now represents "bar/baz"
         */
        ZeroCopyString SplitOffHead(char delimiter);

        bool Equals(const ZeroCopyString& other) const;
        bool Equals(const char* cstr) const;

        bool ValidNumber() const;
        bool ConvertTo_uint32(uint32_t& outValue) const;
        bool ConvertTo_int32(int32_t& outValue) const;
        bool ConvertTo_double(double& outValue) const;
    };
    bool operator==(const ZeroCopyString& lhs, const ZeroCopyString& rhs);
    bool operator!=(const ZeroCopyString& lhs, const ZeroCopyString& rhs);
    bool operator==(const char* lhs, const ZeroCopyString& rhs);
    bool operator!=(const char* lhs, const ZeroCopyString& rhs);
    bool operator==(const ZeroCopyString& lhs, const char* rhs);
    bool operator!=(const ZeroCopyString& lhs, const char* rhs);
}