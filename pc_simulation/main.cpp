
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"
    #include "../src/Support/ConvertHelper.h"

    inline bool StrEqualsIC(const char* strA, const char* strB) { return CharArray::equalsIgnoreCase(strA, strB); }
    
    struct Token {
        const char* text;
        int line;
        int column;
        int itemsInBlock;
    };
    inline bool IsType(const Token& t, const char* str) { return StrEqualsIC(t.text, str); }

    void ReportTokenInfo(const char* msg, const Token& t) {
        std::cout << "Info (line " << t.line << ", column " << t.column << "): " << msg << std::endl;
    }
    void ReportTokenError(const char* msg, const Token& t) {
        std::cout << "Error (line " << t.line << ", column " << t.column << "): " << msg << std::endl;
    }
    void ReportTokenWarning(const char* msg, const Token& t) {
        std::cout << "Warning (line " << t.line << ", column " << t.column << "): " << msg << std::endl;
    }

    

    int CountTokens(char* buffer) {
        int count = 0;
        char* p = buffer;

        while (*p) {
            // Skip whitespace
            while (*p && isspace(static_cast<unsigned char>(*p))) {
                p++;
            }
            if (!*p) break;
            // Start of token
            count++;
            // Advance to next whitespace
            while (*p && !isspace(static_cast<unsigned char>(*p))) {
                p++;
            }
        }
        return count;
    }

    
    

    bool Tokenize(char* buffer, Token* tokens, int tokenCount) {
        int line = 1;
        int column = 1;
        char* p = buffer;

        int tokenIndex = 0;

        while (*p) {
            
            // Skip whitespace
            while (*p && isspace(static_cast<unsigned char>(*p))) {
                if (*p == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                p++;
            }
            // End of string
            if (!*p) break;
            // Start of token
            char* token_start = p;
            int token_column = column;

            while (*p && !isspace(static_cast<unsigned char>(*p))) {
                p++;
                column++;
            }
            // Null-terminate token
            if (*p) {
                *p = '\0';
                p++;
                column++;
            }
            if (tokenIndex == tokenCount) {
                return false; // something went terrible wrong
            }
            tokens[tokenIndex++] = { token_start, line, token_column };
        }
        return true;
    }

    char* ReadFileToMutableBuffer(const std::string& filename, size_t& outSize) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) return nullptr;

        outSize = file.tellg();
        if (outSize == 0) return nullptr;
        file.seekg(0);

        // Allocate mutable buffer (+1 for null terminator if needed)
        char* buffer = new char[outSize + 1];
        file.read(buffer, outSize);
        buffer[outSize] = '\0'; // optional if you need null-terminated text

        return buffer;
    }
    /** special note,
     *  this function do not remove additional \r characters in \r\n \n\r 
     *  it just replaces them with spaces for easier parsing */
    void FixNewLines(char* buffer) {
        char* p = buffer;
        while (*p) {
            if (p[0] == '\r') {
                if (p[1] == '\n') {
                    *p++ = ' ';
                    p++;
                } else {
                    *p++ = '\n';
                }
            } else if (p[0] == '\n' && p[1] == '\r') {
                p++;
                *p++ = ' ';
            } else p++;
        }
    }

    void StripComments(char* buffer) {
        char* p = buffer;
        while (*p) {
            // Handle //
            if (p[0] == '/' && p[1] == '/') {
                *p++ = ' ';
                *p++ = ' ';
                while (*p && *p != '\n' && *p != '\r') {
                    *p++ = ' ';
                }
            }
            // Handle /* ... */
            else if (p[0] == '/' && p[1] == '*') {
                *p++ = ' ';
                *p++ = ' ';
                while (*p && !(p[0] == '*' && p[1] == '/')) {
                    if (*p != '\n' && *p != '\r') {
                        *p = ' ';
                    }
                    p++;
                }
                if (*p) {  // Replace */
                    *p++ = ' ';
                }
                if (*p) {
                    *p++ = ' ';
                }
            }
            else {
                p++;
            }
        }
    }
    #define STRCMP_TRUE 0
    struct Block {
        enum Type { IF, ON } type;
        int tokenIndex;
    };
    int Count_IfTokens(Token* tokens, int tokenCount) {
        int currLevel = 0;
        int maxLevel = 0;
        for (int i = 0; i < tokenCount; i++) {
            if (IsType(tokens[i], "if")) {
                currLevel++;
                if (currLevel > maxLevel) maxLevel = currLevel;
            }
            else if (IsType(tokens[i], "endif")) {
                currLevel--;
            }
        }
        return maxLevel;
    }
    bool VerifyBlocks(Token* tokens, int tokenCount) {
        int onLevel = 0;
        int ifLevel = 0;
        int ifTokenCount = Count_IfTokens(tokens, tokenCount);
        if (ifTokenCount <= 0) ifTokenCount = 1;
        Token* ifStack = new Token[ifTokenCount];
        int ifStackIndex = 0;
        Token lastOn = {nullptr, 0, 0};
        bool otherErrors = false;

        bool expecting_do_then = false;
        int lastControlIndex = 0;

        for (int i = 0; i < tokenCount; i++) {
            Token& token = tokens[i];
            if (IsType(token, "if")) {
                ifLevel++;
                ifStack[ifStackIndex++] = token;
                expecting_do_then = true;
                lastControlIndex = i;
            }
            else if (IsType(token, "elseif")) {
                expecting_do_then = true;
                lastControlIndex = i;
            }
            else if (IsType(token, "on")) {
                if (ifLevel != 0 || onLevel != 0) {
                    ReportTokenError("'on' block cannot be nested", token);
                    otherErrors = true;
                } else {
                    lastOn = token;
                    onLevel++;
                    expecting_do_then = true;
                    lastControlIndex = i;
                }
            }
            else if (IsType(token, "do") || IsType(token, "then")) {
                if (!expecting_do_then) {
                    ReportTokenError("'do'/'then' without preceding 'if' or 'on'", token);
                    otherErrors = true;
                }
                else if (lastControlIndex + 1 == i) {
                    ReportTokenError("Missing condition between 'if/on/elseif' and 'then/do'", token);
                    otherErrors = true;
                }
                expecting_do_then = false;
            }
            else if (IsType(token, "endif")) {
                if (ifLevel == 0) {
                    ReportTokenError("'endif' without matching 'if'", token);
                    otherErrors = true;
                }
                else {
                    if (ifStackIndex > 0)
                        ifStackIndex--;
                    ifLevel--;
                }

                if (expecting_do_then) {
                    ReportTokenError("missing 'do' after last 'if'", token);
                    otherErrors = true;
                }
            }
            else if (IsType(token, "endon")) {
                if (onLevel == 0) {
                    ReportTokenError("'endon' without matching 'on'", token);
                    otherErrors = true;
                } else
                    onLevel--;

                if (expecting_do_then) {
                    ReportTokenError("missing 'do' after last 'on'", token);
                    otherErrors = true;
                }
            } else if (onLevel == 0 && ifLevel == 0) {
                ReportTokenError("tokens cannot be outside root blocks", token);
                otherErrors = true;
            }
        }

        if (ifLevel != 0) {
            for (int i=0;i<ifStackIndex;i++) { // only print last 'errors'
                ReportTokenError("Unmatched 'if' block", ifStack[i]);
            }
            
        }
        if (onLevel != 0) {
            ReportTokenError("Unmatched 'on' block", lastOn);
        }
        delete[] ifStack;
        return (ifLevel == 0) && (onLevel == 0) && (otherErrors == false);
    }
    
    // the function before did overcomplicate things that they dont need to be
    // a easier way is simply just to merge all items between a if and do/then
    // however if a AND/OR token is found between if and do/then
    // it in my first thought i was planning to keep it at place
    // but then i thought again, i could just replace the AND/OR with the optional usage of && and ||
    // then all contents between if and do/then could be easly merged to one line
    void MergeConditions(Token* tokens, int& tokenCount) {
        for (int i = 0; i < tokenCount; i++) {
            if ((IsType(tokens[i], "if") || IsType(tokens[i], "elseif")) == false) continue;

            int start = i + 1;
            int end = -1;
            
            // Find "do" or "then"
            for (int j = start; j < tokenCount; ++j) {
                if (IsType(tokens[j], "do") || IsType(tokens[j], "then")) {
                    end = j;
                    break;
                }
            }
            // as this is actually taken care of in VerifyBlocks this check don't really need to be here
            if (start == end) { 
                ReportTokenError("MergeConditions - empty if condition", tokens[i]);
                continue;
            }

            // malformed if-block, skip (should never happen, as that is taken care of in VerifyBlocks)
            if (end == -1) {
                ReportTokenError("MergeConditions - malformed if", tokens[i]);
                continue;
            }

            // Merge tokens from [start, end)
            char* writePtr = (char*)tokens[start].text;


            for (int j = start+1; j < end; ++j) {
                const Token& token = tokens[j];
                const char* tText = token.text;
                if (IsType(token, "AND")) tText = "&&";
                else if (IsType(token, "OR")) tText = "||";

                strcat(writePtr, tText);
            }

            // Remove tokens between start+1 and end-1
            int shiftCount = end - start - 1;
            if (shiftCount > 0) {
                for (int j = start + 1; j + shiftCount < tokenCount; ++j) {
                    tokens[j] = tokens[j + shiftCount];
                }
                tokenCount -= shiftCount;
            }
        }
    }


    void MergeActions(Token* tokens, int& tokenCount) {
        for (int i = 0; i < tokenCount; i++) {
            const char* text = tokens[i].text;
            if (!(StrEqualsIC(text, "then") ||
                 StrEqualsIC(text, "do") ||
                 StrEqualsIC(text, "and") ||
                 StrEqualsIC(text, "else") ||
                 StrEqualsIC(text, "endif"))) {
                    continue;
            } 
            int start = i + 1;
            int end = -1;
            //ReportTokenInfo("MergeActions found start:", tokens[i]);
            
            for (int j = start; j < tokenCount; ++j) {
                const char* text = tokens[j].text;
                if (tokens[j].line == 23) { std::cout << "@ line 23: " << std::string(tokens[j].text) << std::endl;
                    //return;
                }
                if (!(StrEqualsIC(text, "and") ||
                      StrEqualsIC(text, "if") ||
                      StrEqualsIC(text, "else") ||
                      StrEqualsIC(text, "elseif") ||
                      StrEqualsIC(text, "endon") ||
                      StrEqualsIC(text, "endif"))) continue;

                end = j;
                break;
            }
            if (start == end) {
                // as currently this also checks between 
                // different endif endif and similar situations where actions is not necessary
                // this should not currently be a warning
                // can be taken care of in later checks or at construct time where other checks
                // need to be taken care of beforehand
                //ReportTokenWarning("MergeActions - empty action", tokens[i]);
                continue;
            }

            if (end == -1) {
                //ReportTokenError("MergeActions - malformed action block", tokens[i]);
                continue;
            }

            //ReportTokenInfo("MergeActions found end:", tokens[end]);

            // Merge tokens from [start, end)
            char* writePtr = (char*)tokens[start].text;
            int lineIndex = tokens[start].line;

            for (int j = start+1; j < end; ++j) {
                if (lineIndex != tokens[j].line) { // consider as a separate action token
                    writePtr = (char*)tokens[j].text;
                    lineIndex = tokens[j].line;
                    continue;
                }
                const char* t = tokens[j].text;
                strcat(writePtr, t);
                tokens[j].line = -1; // mark consumed
            }
        }
        // cleanup
        int writeIdx = 0;
        for (int readIdx = 0; readIdx < tokenCount; ++readIdx) {
            if (tokens[readIdx].line == -1) continue;
            const char* tokenText = tokens[readIdx].text;
            // one line action tokens
            if (StrEqualsIC(tokenText, "and")) continue;
            // then and do are no longer nessesary
            //if (StrEqualsIC(tokenText, "then")) continue;
            //if (StrEqualsIC(tokenText, "do")) continue;
            /*if (writeIdx != readIdx)*/ tokens[writeIdx] = tokens[readIdx];
            ++writeIdx;
        }
        tokenCount = writeIdx;
    }

    void CountBlockItems(Token* tokens, int tokenCount) {
        for (int i = 0; i < tokenCount; ++i) {
            Token& token = tokens[i];

            if (IsType(token, "on")) {
                int count = 0;
                for (int j = i + 1; j < tokenCount; ++j) {
                    Token& innerToken = tokens[j];
                    if (IsType(innerToken, "endon")) {
                        break;
                    }
                    count++;
                }
                token.itemsInBlock = count;
            }
            else if (IsType(token, "if")) {
                int count = 1;
                int level = 1;
                for (int j = i + 1; j < tokenCount; ++j) {
                    Token& innerToken = tokens[j];
                    if (IsType(innerToken, "if")) level++;
                    if (IsType(innerToken, "endif")) {
                        level--;
                        if (level == 0) break;
                    }
                    if (level == 1 && (IsType(innerToken, "elseif") || IsType(innerToken, "else"))) {
                        count++;
                    }
                }
                token.itemsInBlock = count;
            }
            else if (IsType(token, "then") || IsType(token, "else")) {//} || IsType(tok, "elseif")) {
                int count = 0;
                int level = 1;
                for (int j = i + 1; j < tokenCount; ++j) {
                    Token& innerToken = tokens[j];
                    if (IsType(innerToken, "if")) level++;
                    if (IsType(innerToken, "endif")) {
                        level--;
                        if (level == 0) break;
                    }
                    if (level == 1 && (IsType(innerToken, "elseif") || IsType(innerToken, "else"))) {
                        break;
                    }
                    if (level == 1) count++;
                }
                token.itemsInBlock = count;
            }
        }
    }

    bool VerifyBlockItemCounts(Token* tokens, int tokenCount) {
        bool anyError = false;
        for (int i = 0; i < tokenCount; ++i) {
            Token& token = tokens[i];
            if ((IsType(token, "then") || IsType(token, "else")) == false) continue;
            if (token.itemsInBlock == 0) {
                ReportTokenError("VerifyBlockItemCounts - empty action(s) block detected", token);
                anyError = true;
            }
        }
        return anyError == false;
    }

    bool VerifyConditionBlocks(Token* tokens, int tokenCount) {
        bool anyError = false;
        for (int i = 0; i < tokenCount; ++i) {
            Token& token = tokens[i];
            if ((IsType(token, "if") || IsType(token, "elseif")) == false) continue;
            const char* conditions = tokens[i+1].text;
            // parantesis could be allowed in a later version
            // do checks here
            // first 'split' by ||
            // and then by &&
            // or as this is only a check, then I could 'split' by either && or ||
            // loop here and verify each condition
            // verify that they only contain one relational operator each
            // verify the left and right operands
            // that they are valid device names and valid const values

        }
        return anyError == false;
    }

    bool ParseRuleSet(Token* tokens, char* fileContents, int tokenCount) {
        if (Tokenize(fileContents, tokens, tokenCount) == false) {
            std::cout << "Error: could not Tokenize" << std::endl;
            return false;
        }
        
        for (int i=0;i<tokenCount;i++) {
            std::cout << "Token("<<i<<"): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ")\t" << tokens[i].text << std::endl;
        }

        std::cout << std::endl << "VerifyBlocks (BetterError): ";
        if (VerifyBlocks(tokens, tokenCount) == false) {
            std::cout << "[FAIL]" << std::endl;
            return false;
        }
        std::cout << "[OK]" << std::endl << std::endl;
        // if here then we can safely parse all blocks

        std::cout << std::endl << "MergeConditions: ";
        MergeConditions(tokens, tokenCount);
        std::cout << "[OK]" << std::endl;
        
        std::cout << std::endl << "MergeActions: ";
        MergeActions(tokens, tokenCount);
        std::cout << "[OK]" << std::endl;
        
        std::cout << std::endl << "CountBlockItems: ";
        CountBlockItems(tokens, tokenCount);
        std::cout << "[OK]" << std::endl;
        
        std::cout << std::endl << "VerifyBlockItemCounts: " << std::endl;
        if (VerifyBlockItemCounts(tokens, tokenCount) == false) {
            std::cout << "[FAIL]" << std::endl;
            return false;
        }
        std::cout << "[OK]" << std::endl;
            
        for (int i=0;i<tokenCount;i++) {
            std::cout << "Token(" << i << "): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ", itemCount:" << tokens[i].itemsInBlock << ")\t" << tokens[i].text << std::endl;
        }

        std::cout << std::endl << "VerifyConditionBlocks: " << std::endl;
        if (VerifyConditionBlocks(tokens, tokenCount) == false) {
            std::cout << "[FAIL]" << std::endl;
            return false;
        }
        return true;
    }

    bool ReadAndParseRuleSetFile(const char* filePath) {
        size_t fileSize;
        char* fileContents = ReadFileToMutableBuffer(filePath, fileSize);
        if (fileContents == nullptr) {
            std::cout << "Error: file could not be read/or is empty" << std::endl;
            return false;
        }
        
        FixNewLines(fileContents);
        StripComments(fileContents); // count_tokens currently wont work without using this
        std::cout << std::endl << "FileContents (after comments removed and newlines fixed): " << std::endl;
        std::cout << fileContents << std::endl;

        int tokenCount = CountTokens(fileContents);
        std::cout << "Token count: " << tokenCount << std::endl;
        Token* tokens = new Token[tokenCount];
        bool anyError = false;
        if (ParseRuleSet(tokens, fileContents, tokenCount)) {
            std::cout << "ParseRuleSet [OK]" << std::endl;
        } else {
            std::cout << "ParseRuleSet [FAIL]" << std::endl;
            anyError = true;
        }
        // dont forget to free/delete
        delete[] fileContents;
        delete[] tokens;
        return anyError == false;
    }

    int main() {
        std::cout << "Running on Windows (MinGW)" << std::endl;
        if (ReadAndParseRuleSetFile("ruleset.txt")) {
            // maybe do something here
        } else {
            // maybe do something here
        }        
    }
