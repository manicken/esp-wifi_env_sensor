
#include "HAL_JSON_RULE_Parser.h"

namespace HAL_JSON {
    namespace Rules {
        static const char* actionStartKeywords[] = {";", "then", "do", "and", "else", "endif", nullptr};
        static const char* actionEndKeywords[] = {";", "and", "if", "else", "elseif", "endon", "endif", nullptr};

        
        
        void Parser::ReportError(const char* msg) {
    #ifdef _WIN32
            std::cout << "Error: " << msg << std::endl;
    #else
            GlobalLogger.Error(F("Rule Set Parse:"), msg);
    #endif
        }

        void Parser::ReportInfo(std::string msg) {
    #ifdef _WIN32
            std::cout << msg;
    #else
            //GlobalLogger.Info(F("Expr Rule Parse:"), msg);
    #endif
        }
        
        


        void Parser::FixNewLines(char* buffer) {
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

        void Parser::StripComments(char* buffer) {
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

        int Parser::CountTokens(char* buffer) {
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

        bool Parser::Tokenize(char* buffer, Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
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
                tokens[tokenIndex++].Set(token_start, line, token_column);
            }
            
            return true;
        }

        std::string Parser::PrintTokens(Tokens& _tokens, bool sub) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            for (int i=0;i<tokenCount;i++) {
                Token& tok = tokens[i];
                if (tok.Merged() && !sub) continue;
                if (IsType(tok, "and")) continue;
                if (IsType(tok, ";")) continue;
                std::string msgLine;
                if (sub) msgLine += "    ";
                //char buffer[64];
                //snprintf(buffer, sizeof(buffer), "addr: %p", (void*)&tokens[i]);
                //std::string memaddr = buffer;
                msgLine +=
                    //memaddr + "  " + 
                    "Token(" + std::to_string(i) + "): " +
                    "(line:" + std::to_string(tok.line) + 
                    ", col:" + std::to_string(tok.column) + 
                    ", itemCount:" + std::to_string(tok.itemsInBlock) + 
                    ", merged:" + std::to_string(tok.merged) + 
                    ", subTokenCount:" + std::to_string(tok.subTokenCount) + 
                    ")";
                if (!sub && tok.subTokenCount > 0) {
                    msgLine += "\n  subTokens:\n";
                    Tokens tokens;
                    tokens.items = &tok;
                    tokens.count = tok.subTokenCount;
                    msgLine += PrintTokens(tokens, true);
                } else {
                    msgLine += "\t" + tok.ToString();// std::string(tok.text);
                }
                msg += msgLine + "\n";
            }
            return msg;
        }

        int Parser::Count_IfTokens(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
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

        bool Parser::VerifyBlocks(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int onLevel = 0;
            int ifLevel = 0;
            int ifTokenCount = Count_IfTokens(_tokens);
            if (ifTokenCount <= 0) ifTokenCount = 1;
            Token** ifStack = new Token*[ifTokenCount]();
            int ifStackIndex = 0;
            Token* lastOn = nullptr;
            bool otherErrors = false;

            bool expecting_do_then = false;
            int lastControlIndex = 0;

            for (int i = 0; i < tokenCount; i++) {
                Token& token = tokens[i];
                if (IsType(token, "if")) {
                    ifLevel++;
                    ifStack[ifStackIndex++] = &token;
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
                        lastOn = &token;
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
                    ReportTokenError("Unmatched 'if' block", *ifStack[i]);
                }
                
            }
            if (onLevel != 0) {
                if (lastOn)
                    ReportTokenError("Unmatched 'on' block: ", *lastOn);
                else
                    ReportError("Unmatched 'on' block: <null>");
            }
            delete[] ifStack;
            return (ifLevel == 0) && (onLevel == 0) && (otherErrors == false);
        }
        int Parser::CountConditionTokens(Tokens& _tokens, int start) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int count = 0;
            for (int i = start; i < tokenCount; i++) {
                if (IsType(tokens[i], "do") || IsType(tokens[i], "then")) {
                    return count;
                }
                else
                    count++;
            }
            return -1; // mean we did not find the do or then token
        }
        
        bool Parser::MergeConditions(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            for (int i = 0; i < tokenCount; i++) {
                if ((IsType(tokens[i], "if") || IsType(tokens[i], "elseif")) == false) continue;
                int conditionTokenCount = CountConditionTokens(_tokens, i+1);
#ifdef _WIN32   
                std::cout << "If case token count: " << conditionTokenCount << "\n";
#endif
                if (conditionTokenCount == -1) return false; // failsafe

                if (conditionTokenCount > 1) { // mergin need to be done
                    i++;
                    tokens[i].InitSubTokens(conditionTokenCount);
                    i += conditionTokenCount; // skip all
                }
            }
            return true;
        }

        bool Parser::MergeActions(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            for (int i = 0; i < tokenCount; ++i) {
                //const char* text = tokens[i].text;

                // Identify start of an action block
                if (!tokens[i].EqualsICAny(actionStartKeywords)) {
                    continue;
                }

                int start = i + 1;
                int end = -1;

                for (int j = start; j < tokenCount; ++j) {
                    //const char* t = tokens[j].text;

                    if (tokens[j].EqualsICAny(actionEndKeywords)) {
                        end = j;
                        break;
                    }
                }

                if (end == -1 || start == end) {
                    continue; // malformed or empty block, skip safely
                }

                int j = start;
                while (j < end) {
                    int currentLine = tokens[j].line;
                    int lineTokenCount = 0;

                    // Count how many tokens are on this line
                    while (j + lineTokenCount < end &&
                        tokens[j + lineTokenCount].line == currentLine) {
                        lineTokenCount++;
                    }

                    if (lineTokenCount > 1) {
                        tokens[j].InitSubTokens(lineTokenCount);
                        j += lineTokenCount;
                    } else {
                        j++; // single token, skip merging
                    }
                }
                i = end - 1; // continue after this block
            }
            return true;
        }

        void Parser::CountBlockItems(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            for (int i = 0; i < tokenCount; ++i) {
                Token& token = tokens[i];

                if (IsType(token, "on")) {
                    int count = 0;
                    for (int j = i + 1; j < tokenCount; ++j) {
                        Token& innerToken = tokens[j];
                        if (innerToken.Merged()) continue;
                        if (IsType(innerToken, "and")) continue;
                        if (IsType(innerToken, ";")) continue;
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
                        if (innerToken.Merged()) continue;
                        if (IsType(innerToken, "and")) continue;
                        if (IsType(innerToken, ";")) continue;
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
                        if (innerToken.Merged()) continue;
                        if (IsType(innerToken, "and")) continue;
                        if (IsType(innerToken, ";")) continue;
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

        bool Parser::EnsureActionBlocksContainItems(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                Token& token = tokens[i];
                if ((IsType(token, "then") || IsType(token, "else")) == false) continue;
                if (token.itemsInBlock == 0) {
                    ReportTokenError("EnsureActionBlocksContainItems - empty action(s) block detected", token);
                    anyError = true;
                }
            }
            return anyError == false;
        }

        bool Parser::VerifyConditionBlocks(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                Token& token = tokens[i];
                if ((IsType(token, "if") || IsType(token, "elseif")) == false) continue;
                //const char* conditions = tokens[i+1].text;
                Tokens conditions;
                conditions.items = &tokens[i+1];
                if (tokens[i+1].subTokenCount != 0) {
                    conditions.count = tokens[i+1].subTokenCount;
                } else {
                    conditions.count = 1;
                }
                ReportInfo("\n"); // newline
                ReportInfo(conditions.ToString());
                ReportInfo("\n"); // newline
                
                if (Expressions::ValidateExpression(conditions) == false) anyError = true;

            }
            return anyError == false;
        }

        bool Parser::ParseRuleSet(char* fileContents, Tokens& _tokens) {
            
            if (Tokenize(fileContents, _tokens) == false) {
                ReportInfo("Error: could not Tokenize\n");
                //std::cout << "Error: could not Tokenize" << std::endl;
                return false;
            }
#ifdef _WIN32
            std::cout << "**********************************************************************************\n";
            std::cout << "*                            RAW TOKEN LIST                                      *\n";
            std::cout << "**********************************************************************************\n";
#endif
            ReportInfo(PrintTokens(_tokens) + "\n");

            ReportInfo("\nVerifyBlocks (BetterError): ");
            if (VerifyBlocks(_tokens) == false) {
                ReportInfo("[FAIL]\n");
                return false;
            }
            ReportInfo("[OK]\n");
            // if here then we can safely parse all blocks

            ReportInfo("\nMergeConditions: ");
            MergeConditions(_tokens);
            ReportInfo("[OK]\n");
            
            ReportInfo("\nMergeActions: ");
            MergeActions(_tokens);
            ReportInfo("[OK]\n");
            
            ReportInfo("\nCountBlockItems: ");
            CountBlockItems(_tokens);
            ReportInfo("[OK]\n");
            
            ReportInfo("\nEnsureActionBlocksContainItems: ");
            if (EnsureActionBlocksContainItems(_tokens) == false) {
                ReportInfo("[FAIL]\n");
                return false;
            }
            ReportInfo("[OK]\n");

#ifdef _WIN32
            std::cout << "**********************************************************************************\n";
            std::cout << "*                            PARSED TOKEN LIST                                   *\n";
            std::cout << "**********************************************************************************\n";
#endif
            ReportInfo(PrintTokens(_tokens) + "\n");

            ReportInfo("\nVerifyConditionBlocks: ");
            if (VerifyConditionBlocks(_tokens) == false) {
                ReportInfo("[FAIL]\n");
                return false;
            }
            return true;
        }

        bool Parser::ReadAndParseRuleSetFile(const char* filePath) {
            size_t fileSize;
            char* fileContents;// = ReadFileToMutableBuffer(filePath, fileSize);
            if (LittleFS_ext::load_from_file(filePath, &fileContents, &fileSize) == false) {
                ReportInfo("Error: file could not be read/or is empty\n");
                return false;
            }
            
            FixNewLines(fileContents);
            // replaces all comments with whitespace
            // make it much simpler to parse the contents 
            StripComments(fileContents);
            //ReportInfo("\nFileContents (after comments removed and newlines fixed): ");
            //ReportInfo(fileContents);

            int tokenCount = CountTokens(fileContents);
            ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
            Tokens tokens(tokenCount);
            
            bool anyError = false;
            if (ParseRuleSet(fileContents, tokens)) {
                ReportInfo("ParseRuleSet [OK]\n");
            } else {
                ReportInfo("ParseRuleSet [FAIL]\n");
                anyError = true;
            }
            // dont forget to free/delete
            delete[] fileContents;
            return anyError == false;
        }
    }
}