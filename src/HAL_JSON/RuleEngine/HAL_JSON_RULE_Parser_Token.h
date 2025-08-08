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
        /** 
         * for the fundamentala types
         * this is set in the tokenize function.
         */
        enum class TokenType {
            /** used to make it easier to see unset tokens, is also used as terminator item when defining a list of token types */
            NotSet, 
            On,
            EndOn,
            If,
            EndIf,
            /** Do/Then token */
            Then,
            Else,
            ElseIf,
            IfCondition,
            /** used to mark actions, note if a action spans multiple tokens only the root is marked, the rest is marked Merged */
            Action,
            Merged,
            /** used both as a conditional and/&& and as a action separator in one line scripts */
            And,
            /** specific action separator currently defined as ; should be marked as Ignored When consumed*/
            ActionSeparator,
            /** used to split actions into multiple lines currently \ (note it need to have a space before and whitespace after), should be marked as Ignored When consumed*/
            ActionJoiner,
            /** marks the token to be ignored in futher parsing, should be marked as Ignored When consumed*/
            Ignore
        };

        const char* TokenTypeToString(TokenType type);
        TokenType GetFundamentalTokenType(const char* str);

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
            TokenType type;
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
            /** 
             * used when 'grouping' tokens that actually belong to the same construct
             * note currently the valid types are only constructTypes:
             * TokenType::IfCondition
             * TokenType::Action
             */
            void InitSubTokens(int size, TokenType constructType);
            /** 
             * return true if this token is a subtoken of another token.
             * Used to skip it during post-processing checks.
             * basically (merged && subTokenCount == 0)
             */
            bool Merged() const;
            bool AnyType(const TokenType* candidates);
            ~Token();
        };
        /** Tokens are considered identical if their 'start' pointers are the same */
        inline bool operator==(const Token& lhs, const Token& rhs) { return lhs.start == rhs.start; }
        /** Tokens are considered not identical if their 'start' pointers are not the same */
        inline bool operator!=(const Token& lhs, const Token& rhs) { return lhs.start != rhs.start; }
        struct Tokens {
        private:
            bool zeroCopy;
        public:
            /** used when there is a situation where the first token need to be splitted
             * i.e. for example when parsing 
             * assigment action RHS expressions 
             * by the Expression_Parser such as
             * someVar =5 or someVar +=5
             * and also
             * someVar=5 or someVar+=5
             */
            const char* firstTokenStartOffset;
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