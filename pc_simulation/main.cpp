
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"
    #include "../src/Support/ConvertHelper.h"

    void ReportError(const char* msg, int line, int column) {
        std::cout << "Error (line " << line << ", column " << column << "): " << msg << std::endl;
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
    struct Token {
        const char* text;
        int line;
        int column;
    };
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
        int count = 0;
        for (int i = 0; i < tokenCount; i++) {
            if (CharArray::equalsIgnoreCase(tokens[i].text, "if"))
                count++;
        }
        return count;
    }
    bool VerifyBlocks(Token* tokens, int tokenCount) {
        int onLevel = 0;
        int ifLevel = 0;
        int ifTokenCount = Count_IfTokens(tokens, tokenCount);
        Token* ifStack = new Token[ifTokenCount];
        int ifStackIndex = 0;
        Token lastOn = {nullptr, 0, 0};
        bool otherErrors = false;

        bool expecting_do = false;

        for (int i = 0; i < tokenCount; i++) {
            if (CharArray::equalsIgnoreCase(tokens[i].text, "if")) {
                ifLevel++;
                ifStack[ifStackIndex++] = tokens[i];
                expecting_do = true;
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "on")) {
                if (ifLevel != 0 || onLevel != 0) {
                    ReportError("'on' block cannot be nested", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                } else {
                    lastOn = tokens[i];
                    onLevel++;
                    expecting_do = true;
                }
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "do")) {
                if (!expecting_do) {
                    ReportError("'do' without preceding 'if' or 'on'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                }
                expecting_do = false;
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "endif")) {
                if (ifLevel == 0) {
                    ReportError("'endif' without matching 'if'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                }
                else {
                    if (ifStackIndex > 0)
                        ifStackIndex--;
                    ifLevel--;
                }

                if (expecting_do) {
                    ReportError("missing 'do' after last 'if'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                }
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "endon")) {
                if (onLevel == 0) {
                    ReportError("'endon' without matching 'on'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                } else
                    onLevel--;

                if (expecting_do) {
                    ReportError("missing 'do' after last 'on'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                }
            } else if (onLevel == 0 && ifLevel == 0) {
                ReportError("tokens cannot be outside blocks", tokens[i].line, tokens[i].column);
                otherErrors = true;
            }
        }

        if (ifLevel != 0) {
            for (int i=0;i<ifStackIndex;i++) { // only print last 'errors'
                ReportError("Unmatched 'if' block", ifStack[i].line, ifStack[i].column);
            }
            
        }
        if (onLevel != 0) {
            ReportError("Unmatched 'on' block", lastOn.line, lastOn.column);
        }
        delete[] ifStack;
        return (ifLevel == 0) && (onLevel == 0) && (otherErrors == false);
    }

    void MergeConditions(Token* tokens, int& tokenCount) {

        const char* ops[] = {">=", "<=", "==", "!=", ">", "<"};
        const size_t ops_len[] = {2, 2, 2, 2, 1, 1};
        const int opCount = sizeof(ops) / sizeof(ops[0]);

        for (int i = 0; i < tokenCount; ++i) {
            size_t strLen = strlen(tokens[i].text);
            for (int j = 0; j < opCount; j++) {
                size_t opLen = ops_len[j];
                if ((strncmp(tokens[i].text, ops[j], opLen) == 0)) {
                    if ((strLen > opLen)) {
                        std::cout << "Merge opportunity, found token (" << tokens[i].line << "):[" << tokens[i].text << "]" << std::endl;
                    }
                    break;
                }
            }
        }
    }


    int main() {
        std::cout << "Running on Windows (MinGW)" << std::endl;

        size_t fileSize;
        char* fileContents = ReadFileToMutableBuffer("ruleset.txt", fileSize);
        
        FixNewLines(fileContents);
        StripComments(fileContents); // count_tokens currently wont work without using this
        std::cout << std::endl << "FileContents (after comments removed and newlines fixed): " << std::endl;
        std::cout << fileContents << std::endl;

        int tokenCount = CountTokens(fileContents);
        std::cout << "Token count: " << tokenCount << std::endl;
        Token* tokens = new Token[tokenCount];
        if (Tokenize(fileContents, tokens, tokenCount) == true) {
            for (int i=0;i<tokenCount;i++) {
                std::cout << "Token("<<i<<"): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ")\t" << tokens[i].text << std::endl;
            }

            std::cout << std::endl << "VerifyBlocksBetterError: " << std::endl;
            if (VerifyBlocks(tokens, tokenCount) == true) {
                std::cout << "[OK]" << std::endl;
                // if here then we can safely parse all blocks
                MergeConditions(tokens, tokenCount);
                // debug print
                for (int i=0;i<tokenCount;i++) {
                    std::cout << "Token("<<i<<"): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ")\t" << tokens[i].text << std::endl;
                }
            }
            else {
                std::cout << "[FAIL]" << std::endl;
            }
        } else {
            std::cout << "something is terrible wrong" << std::endl;
        }
        // dont forget to free/delete
        delete[] fileContents;
        delete[] tokens;
    }
