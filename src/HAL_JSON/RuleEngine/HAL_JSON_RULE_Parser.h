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
#include "../../Support/ZeroCopyString.h"

namespace HAL_JSON {
    namespace Rules {
        
        class Parser {
        private:
            
            static void ReportError(const char* msg);
            static void ReportInfo(std::string msg);
            static inline bool IsType(const Token& t, const char* str) { return StrEqualsIC(t.text, str); }

            static void ReportTokenInfo(const char* msg, const Token& t);
            static void ReportTokenError(const char* msg, const Token& t);
            static void ReportTokenWarning(const char* msg, const Token& t);
            /** special note,
              *  this function do not remove additional \r characters in \r\n \n\r 
              *  it just replaces them with spaces for easier parsing 
              */
            static void FixNewLines(char* buffer);
            static void StripComments(char* buffer);

            static int CountTokens(char* buffer);
            static bool Tokenize(char* buffer, Token* tokens, int tokenCount);

            static std::string PrintTokens(Token* tokens, int tokenCount, bool sub = false);

            /** used by VerifyBlocks */
            static int Count_IfTokens(Token* tokens, int tokenCount);

            /** verify if/on blocks so that they follow the structure
             * on/if <trigger>/<condition> do/then <action(s)> endon/endif
             * it also verify that on blocks do only contain if blocks
             */
            static bool VerifyBlocks(Token* tokens, int tokenCount);
            static int CountConditionTokens(Token* tokens, int start, int tokenCount);

            /** 
             * merge Conditions into one token for easier parse,
             * if a AND/OR token is found they are 
             * replaced by && and || respective 
             */
            static bool MergeConditions(Token* tokens, int& tokenCount);

            /** merge actions so that each action 'line' is in one token for easier parse */
            static bool MergeActions(Token* tokens, int& tokenCount);

            /** this is used together with EnsureActionBlocksContainItems */
            static void CountBlockItems(Token* tokens, int tokenCount);
            static bool EnsureActionBlocksContainItems(Token* tokens, int tokenCount);

            static bool VerifyConditionBlocks(Token* tokens, int tokenCount);
            static bool ParseRuleSet(Token* tokens, char* fileContents, int tokenCount);
            
        public:
            static bool ReadAndParseRuleSetFile(const char* filePath);

        };
    }
}