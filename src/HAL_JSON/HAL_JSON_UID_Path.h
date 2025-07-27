
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
#include "../Support/ZeroCopyString.h"
#include "../Support/ConvertHelper.h"
#include "../Support/Logger.h"
#include "HAL_JSON_UID.h"

namespace HAL_JSON {

    

    class UIDPath {
    private:
        HAL_UID* items = nullptr;

        //HAL_UID* items = nullptr; // future implementation so that encodeUID and decodeUID won't be needed
        uint32_t itemCount = 0;
        uint32_t currentItemIndex = 0;

    public:
        enum class ToStringType {
            String,
            Raw
        };

        UIDPath();
        ~UIDPath();
        UIDPath(const char* uidStr);
        UIDPath(const ZeroCopyString& uidzcStr);
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
        HAL_UID getCurrentUID();
        HAL_UID resetAndGetFirst();
        HAL_UID getNextUID();
        HAL_UID peekNextUID();
        bool isLast();

        std::string ToString(ToStringType type = ToStringType::String);

        //uint64_t byIndex(uint32_t index); // usage of this function don't make any sense
    };

}