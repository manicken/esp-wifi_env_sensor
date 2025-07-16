
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"
    #include "../src/Support/ConvertHelper.h"

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
        std::string ToString() {
            return "line:" + std::to_string(line) + ", col:" + std::to_string(column) + " " + std::string(text);
        }
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
    bool VerifyBlocks(Token* tokens, int tokenCount) {
        int onLevel = 0;
        int ifLevel = 0;
        std::vector<Token> ifStack;
        Token lastOn = {nullptr, 0, 0};
        bool otherErrors = false;

        bool expecting_do = false;

        for (int i = 0; i < tokenCount; i++) {
            if (strcmp(tokens[i].text, "if") == STRCMP_TRUE) {
                ifLevel++;
                ifStack.push_back(tokens[i]);
                expecting_do = true;
            }
            else if (strcmp(tokens[i].text, "on") == STRCMP_TRUE) {
                if (ifLevel != 0 || onLevel != 0) {
                    std::cout << "Error: 'on' block cannot be nested @ line:" << tokens[i].line << ", col:" << tokens[i].column<< std::endl;
                    otherErrors = true;
                } else {
                    lastOn = tokens[i];
                    onLevel++;
                    expecting_do = true;
                }
            }
            else if (strcmp(tokens[i].text, "do") == STRCMP_TRUE) {
                if (!expecting_do) {
                    std::cout << "Error: 'do' without preceding 'if' or 'on' @ line:" << tokens[i].line << ", col:" << tokens[i].column<< std::endl;
                    otherErrors = true;
                }
                expecting_do = false;
            }
            else if (strcmp(tokens[i].text, "endif") == STRCMP_TRUE) {
                if (ifLevel == 0) {// || (block_stack.top() != BlockType::IF)) {
                    std::cout << "Error: 'endif' without matching 'if' @ line:" << tokens[i].line << ", col:" << tokens[i].column << std::endl;
                    otherErrors = true;
                }
                else {
                    ifStack.pop_back();
                    ifLevel--;
                }

                if (expecting_do) {
                    std::cout << "Error: missing 'do' after last 'if' @ line:" << tokens[i].line << ", col:" << tokens[i].column << std::endl;
                    otherErrors = true;
                }
            }
            else if (strcmp(tokens[i].text, "endon") == STRCMP_TRUE) {
                if (onLevel == 0) {
                    std::cout << "Error: 'endon' without matching 'on' @ line:" << tokens[i].line << ", col:" << tokens[i].column << std::endl;
                    otherErrors = true;
                } else
                    onLevel--;

                if (expecting_do) {
                    std::cout << "Error: missing 'do' after last 'on' @ line:" << tokens[i].line << ", col:" << tokens[i].column << std::endl;
                    otherErrors = true;
                }
            }
        }

        if (ifLevel != 0) {
            //std::cout << "Error: Unmatched 'if' blocks detected:" << ifLevel << std::endl;
            for (auto& ifItem : ifStack) {
                std::cout << "Error: Unmatched 'if' block @ line:" << ifItem.line << ", col:" << ifItem.column << std::endl;
            }
            
        }
        if (onLevel != 0)
            std::cout << "Error: Unmatched 'on' block @ line:" << lastOn.line << ", col:" << lastOn.column << std::endl;

        return (ifLevel == 0) && (onLevel == 0) && (otherErrors == false);
    }

    struct UID_Test {
        union {
            uint64_t uidVal;
            char uidStr[8];
        };
    };

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
