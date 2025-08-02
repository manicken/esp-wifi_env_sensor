#pragma once

#include <Arduino.h> // Needed for String class
#ifdef _WIN32
#include <iostream> // including this take 209512 bytes flash
#endif
#include <string>
#include <cstdint>

#include <vector>
#include <stack>

#include "HAL_JSON_RULE_Expression_Parser.h"
#include "HAL_JSON_RULE_Engine.h"
#include "../../Support/Logger.h"
#include "../../Support/ConvertHelper.h"
#include "../../Support/CharArrayHelpers.h"
#include "../../Support/ZeroCopyString.h"

namespace HAL_JSON {
    namespace Rules {
        struct Token {
            
            /**
             * note this is only a pointer to a existing string in memory, 
             * so no freeing should be done 
             * TODO change it to ZeroCopyString to avoid confusion
             */
            const char* text;
            /** this only stores the pointer to where the subTokens block start */
            Token* subTokens;
            int subTokenCount;
            int line;
            int column;
            int itemsInBlock;
            /** 
             * Set to true if this token is a subtoken of another token.
             * Used to skip it during post-processing checks.
             */
            bool merged;
            Token();

            /* no copy/move constructors/assigments needed*/
            Token(Token&) = delete;          // no copy constructor
            Token& operator=(const Token&) = delete; // no copy assignment
            Token(Token&& other) = delete;           // no move constructor
            Token& operator=(Token&& other) = delete; // no move assignment

            void Set(const char* text, int line, int column);
            void InitSubTokens(int size);
            /** this checks if the current token is 
             * 'merged into'/'belongs to' annother token as a subToken, 
             * and if it can be ignored by further parsing
             */
            bool Merged();
            ~Token();
        };
    }
}