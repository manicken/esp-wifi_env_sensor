#pragma once

#include <Arduino.h> // Needed for String class
#ifdef _WIN32
#include <iostream> // including this take 209512 bytes flash
#endif
#include <string>
#include <cstdint>

#include <vector>
#include <stack>

#include "../../Support/Logger.h"
#include "../../Support/ConvertHelper.h"
#include "../../Support/CharArrayHelpers.h"
#include "../HAL_JSON_ZeroCopyString.h"

namespace HAL_JSON {
    namespace Rules {


        struct Token : public ZeroCopyString {
            using ZeroCopyString::ZeroCopyString;

            /**
             * note this is only a pointer to a existing string in memory, 
             * so no freeing should be done 
             * TODO change it to ZeroCopyString to avoid confusion
             */
            //const char* text;
            /** this only stores the pointer to where the subTokens block start */
            //Tokens subTokens; // if used like this is the default constructor called
            //Token* subTokens;
            int subTokenCount;
            int line;
            int column;
            int itemsInBlock;
            bool isAction;
            /** 
             * Set to true if this token is a subtoken of another token.
             * Used to skip it during post-processing checks.
             * basically (merged && subTokenCount == 0)
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
            /** 
             * return true if this token is a subtoken of another token.
             * Used to skip it during post-processing checks.
             * basically (merged && subTokenCount == 0)
             */
            bool Merged();
            ~Token();
        };

        struct Tokens {
        private:
            bool zeroCopy;
        public:
            Token* items;
            int count;
            /** this will initialize this instance as a zeroCopyPointer storage */
            Tokens();
            /** this will initialize this instance owned Token storage */
            Tokens(int count);
            ~Tokens();

            Tokens(Tokens&) = delete;          // no copy constructor
            Tokens& operator=(const Tokens&) = delete; // no copy assignment
            Tokens(Tokens&& other) = delete;           // no move constructor
            Tokens& operator=(Tokens&& other) = delete; // no move assignment
            std::string ToString();
        };

        void ReportTokenInfo(const Token& t, const char* msg, const char* param = nullptr);
        void ReportTokenError(const Token& t, const char* msg, const char* param = nullptr);
        void ReportTokenWarning(const Token& t, const char* msg, const char* param = nullptr);
        
        std::string PrintTokens(Tokens& tokens, bool sub = false);
    }
}