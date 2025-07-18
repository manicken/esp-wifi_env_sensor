
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"
    #include "../src/Support/ConvertHelper.h"

    void ReportInfo(const char* msg, int line, int column) {
        std::cout << "Info (line " << line << ", column " << column << "): " << msg << std::endl;
    }
    void ReportError(const char* msg, int line, int column) {
        std::cout << "Error (line " << line << ", column " << column << "): " << msg << std::endl;
    }
    void ReportWarning(const char* msg, int line, int column) {
        std::cout << "Warning (line " << line << ", column " << column << "): " << msg << std::endl;
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

        bool expecting_do_then = false;

        for (int i = 0; i < tokenCount; i++) {
            if (CharArray::equalsIgnoreCase(tokens[i].text, "if")) {
                ifLevel++;
                ifStack[ifStackIndex++] = tokens[i];
                expecting_do_then = true;
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "elseif")) {
                expecting_do_then = true;
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "on")) {
                if (ifLevel != 0 || onLevel != 0) {
                    ReportError("'on' block cannot be nested", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                } else {
                    lastOn = tokens[i];
                    onLevel++;
                    expecting_do_then = true;
                }
            }
            else if (CharArray::equalsIgnoreCase(tokens[i].text, "do") || CharArray::equalsIgnoreCase(tokens[i].text, "then")) {
                if (!expecting_do_then) {
                    ReportError("'do'/'then' without preceding 'if' or 'on'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                }
                expecting_do_then = false;
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

                if (expecting_do_then) {
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

                if (expecting_do_then) {
                    ReportError("missing 'do' after last 'on'", tokens[i].line, tokens[i].column);
                    otherErrors = true;
                }
            } else if (onLevel == 0 && ifLevel == 0) {
                ReportError("tokens cannot be outside root blocks", tokens[i].line, tokens[i].column);
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
    
    // the function before did overcomplicate things that they dont need to be
    // a easier way is simply just to merge all items between a if and do/then
    // however if a AND/OR token is found between if and do/then
    // it in my first thought i was planning to keep it at place
    // but then i thought again, i could just replace the AND/OR with the optional usage of && and ||
    // then all contents between if and do/then could be easly merged to one line
    void MergeConditions(Token* tokens, int& tokenCount) {
        for (int i = 0; i < tokenCount; i++) {
            if (CharArray::equalsIgnoreCase(tokens[i].text, "if") == false) continue;

            int start = i + 1;
            int end = -1;
            
            // Find "do" or "then"
            for (int j = start; j < tokenCount; ++j) {
                if (CharArray::equalsIgnoreCase(tokens[j].text, "do") || CharArray::equalsIgnoreCase(tokens[j].text, "then")) {
                    end = j;
                    break;
                }
            }
            if (start == end) {
                ReportWarning("MergeConditions - empty if condition", tokens[i].line, tokens[i].column);
                continue;
            }

            // malformed if-block, skip (should never happen, as that is taken care of in VerifyBlocks)
            if (end == -1) {
                ReportError("MergeConditions - malformed if", tokens[i].line, tokens[i].column);
                continue;
            }

            // Merge tokens from [start, end)
            char* writePtr = (char*)tokens[start].text;


            for (int j = start+1; j < end; ++j) {
                const char* t = tokens[j].text;
                if (CharArray::equalsIgnoreCase(t, "AND")) t = "&&";
                else if (CharArray::equalsIgnoreCase(t, "OR")) t = "||";

                strcat(writePtr, t);
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
            if (!(CharArray::equalsIgnoreCase(text, "then") ||
                 CharArray::equalsIgnoreCase(text, "do") ||
                 CharArray::equalsIgnoreCase(text, "and") ||
                 CharArray::equalsIgnoreCase(text, "else") ||
                 CharArray::equalsIgnoreCase(text, "endif"))) {
                    continue;
            } 
            int start = i + 1;
            int end = -1;
            ReportInfo("MergeActions found start:", tokens[i].line, tokens[i].column);
            
            for (int j = start; j < tokenCount; ++j) {
                const char* text = tokens[j].text;
                if (tokens[j].line == 23) { std::cout << "@ line 23: " << std::string(tokens[j].text) << std::endl;
                    //return;
                }
                if (!(CharArray::equalsIgnoreCase(text, "and") ||
                      CharArray::equalsIgnoreCase(text, "if") ||
                      CharArray::equalsIgnoreCase(text, "else") ||
                      CharArray::equalsIgnoreCase(text, "elseif") ||
                      CharArray::equalsIgnoreCase(text, "endon") ||
                      CharArray::equalsIgnoreCase(text, "endif"))) continue;

                end = j;
                break;
            }
            if (start == end) {
                //  this is not even a warning
                ReportInfo("MergeActions - empty action", tokens[i].line, tokens[i].column);
                continue;
            }

            if (end == -1) {
                ReportError("MergeActions - malformed action block", tokens[i].line, tokens[i].column);
                continue;
            }

            ReportInfo("MergeActions found end:", tokens[end].line, tokens[end].column);

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
            // one line action tokens
            if (CharArray::equalsIgnoreCase(tokens[readIdx].text, "and")) continue;
            // then and do are no longer nessesary
            if (CharArray::equalsIgnoreCase(tokens[readIdx].text, "then")) continue;
            if (CharArray::equalsIgnoreCase(tokens[readIdx].text, "do")) continue;
            /*if (writeIdx != readIdx)*/ tokens[writeIdx] = tokens[readIdx];
            ++writeIdx;
        }
        tokenCount = writeIdx;
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
                std::cout << "[OK]" << std::endl << std::endl;
                // if here then we can safely parse all blocks
                std::cout << std::endl << "Mergin Conditions:" << std::endl;
                MergeConditions(tokens, tokenCount);
                std::cout << "[OK]" << std::endl;
                std::cout << std::endl << "Mergin Actions:" << std::endl;
                MergeActions(tokens, tokenCount);
                std::cout << "[OK]" << std::endl;
                std::cout << std::endl;
                // debug print
                //int count = 0;
                for (int i=0;i<tokenCount;i++) {
                    //if (tokens[i].line == -1) continue;
                    //count++;
                    std::cout << "Token("<<i<<"): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ")\t" << tokens[i].text << std::endl;
                }
                //std::cout << "Token count after action merge: " << count << std::endl;
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
