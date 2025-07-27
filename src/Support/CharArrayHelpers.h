#pragma once

#ifndef _WIN32
#include <Arduino.h>
#endif
#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

namespace CharArray {

    


    uint32_t countChar(const char* str, char ch);
    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount);
    
    const char** getPointers(const char* str, char ch, uint32_t& outCount);
    /** case insensitive compare */
    bool equalsIgnoreCase(const char* a, const char* b);

    int count_tokens(const char* str, const char* delimiters);
    char *my_strtok(char **str, const char *delimiters);

    bool EndsWith(const char* str, const char* suffix);
    /*
    struct StringView {
        const char* ptr;
        size_t len;

        StringView();
        StringView(const char* p, size_t l);

        bool empty() const;

        std::string toString() const;
    };*/

}