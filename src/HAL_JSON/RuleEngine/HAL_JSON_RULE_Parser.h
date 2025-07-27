#pragma once

#include <Arduino.h> // Needed for String class
#ifdef _WIN32
#include <iostream> // including this take 209512 bytes flash
#endif
#include <string>
#include <cstdint>

#include <vector>
#include <stack>
#include "HAL_JSON_RULE_Engine.h"
#include "../../Support/ConvertHelper.h"
#include "../../Support/CharArrayHelpers.h"
#include "../../Support/ZeroCopyString.h"

namespace HAL_JSON {

    struct Token {
        const char* text;
        int line;
        int column;
        int itemsInBlock;
    };

    inline bool IsType(const Token& t, const char* str) { return StrEqualsIC(t.text, str); }

    void ReportTokenInfo(const char* msg, const Token& t);
    void ReportTokenError(const char* msg, const Token& t);
    void ReportTokenWarning(const char* msg, const Token& t);
    /** special note,
      *  this function do not remove additional \r characters in \r\n \n\r 
      *  it just replaces them with spaces for easier parsing 
      */
    void FixNewLines(char* buffer);
    void StripComments(char* buffer);

    int CountTokens(char* buffer);
    bool Tokenize(char* buffer, Token* tokens, int tokenCount);
    
    /** used by VerifyBlocks */
    int Count_IfTokens(Token* tokens, int tokenCount);
    /** verify if/on blocks so that they follow the structure
     * on/if <trigger>/<condition> do/then <action(s)> endon/endif
     * it also verify that on blocks do only contain if blocks
     */
    bool VerifyBlocks(Token* tokens, int tokenCount);
    /** 
     * merge Conditions into one token for easier parse,
     * if a AND/OR token is found they are 
     * replaced by && and || respective 
     */
    void MergeConditions(Token* tokens, int& tokenCount);
    /** merge actions so that each action 'line' is in one token for easier parse */
    void MergeActions(Token* tokens, int& tokenCount);
    /** this is used together with EnsureActionBlocksContainItems */
    void CountBlockItems(Token* tokens, int tokenCount);
    bool EnsureActionBlocksContainItems(Token* tokens, int tokenCount);

}