#pragma once

#include <Arduino.h> // Needed for String class
#ifdef _WIN32
#include <iostream> // including this take 209512 bytes flash

#endif
#include <string>
#include <cstdint>


#include <vector>
#include <stack>
#include "HAL_JSON_RULE_Parser_Token.h"
#include "HAL_JSON_RULE_Expression_Parser.h"
#include "HAL_JSON_RULE_Engine.h"
#include "../../Support/ConvertHelper.h"
#include "../../Support/CharArrayHelpers.h"
#include "../HAL_JSON_ZeroCopyString.h"

namespace HAL_JSON {
    namespace Rules {
        
        class Parser {
        private:
            
            static void ReportError(const char* msg);
            static void ReportInfo(std::string msg);
            static inline bool IsType(const Token& t, const char* str) { return t.EqualsIC(str); }

            /** special note,
              *  this function do not remove additional \r characters in \r\n \n\r 
              *  it just replaces them with spaces for easier parsing 
              */
            static void FixNewLines(char* buffer);
            static void StripComments(char* buffer);

            static int CountTokens(char* buffer);
            static bool Tokenize(char* buffer, Tokens& tokens);

            /** used by VerifyBlocks */
            static int Count_IfTokens(Tokens& tokens);

            /** verify if/on blocks so that they follow the structure
             * on/if <trigger>/<condition> do/then <action(s)> endon/endif
             * it also verify that on blocks do only contain if blocks
             */
            static bool VerifyBlocks(Tokens& tokens);
            static int CountConditionTokens(Tokens& tokens, int start);

            /** 
             * merge Conditions into one token for easier parse,
             * if a AND/OR token is found they are 
             * replaced by && and || respective 
             */
            static bool MergeConditions(Tokens& tokens);

            /** merge actions so that each action 'line'
             *  is in one token for easier parse */
            static bool MergeActions(Tokens& tokens);
            /** merge actions so that each action 'line'
             *  is in one token for easier parse 
             *  this is a variant to MergeActions but
             *  allows the use of \ to make multiline spanning actions
             */
            static bool MergeActions2(Tokens& tokens);

            /** this is used together with EnsureActionBlocksContainItems */
            static void CountBlockItems(Tokens& tokens);
            static bool EnsureActionBlocksContainItems(Tokens& tokens);

            static bool VerifyConditionBlocks(Tokens& tokens);
            static bool VerifyActionBlocks(Tokens& tokens);
            static bool ParseRuleSet(char* fileContents,Tokens& tokens);
            
        public:
            static bool ReadAndParseRuleSetFile(const char* filePath);

        };
    }
}