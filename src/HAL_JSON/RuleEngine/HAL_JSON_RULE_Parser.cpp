
#include "HAL_JSON_RULE_Parser.h"

namespace HAL_JSON {
    namespace Rules {
        static const char* actionStartKeywords[] = {";", "then", "do", "and", "else", "endif", nullptr};
        static const char* actionEndKeywords[] = {";", "and", "if", "else", "elseif", "endon", "endif", nullptr};
        //static const char* actionUnsupportedAssignKeywords[] {"+=", "-=", "*=", "/=", "%=", "&=", "|=", nullptr};
       // static const char* actionAssignKeywords[] {"=", nullptr};

        
        
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
                        ReportTokenError(token, "'on' block cannot be nested");
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
                        ReportTokenError(token, "'do'/'then' without preceding 'if' or 'on'");
                        otherErrors = true;
                    }
                    else if (lastControlIndex + 1 == i) {
                        ReportTokenError(token, "Missing condition between 'if/on/elseif' and 'then/do'");
                        otherErrors = true;
                    }
                    expecting_do_then = false;
                }
                else if (IsType(token, "endif")) {
                    if (ifLevel == 0) {
                        ReportTokenError(token, "'endif' without matching 'if'");
                        otherErrors = true;
                    }
                    else {
                        if (ifStackIndex > 0)
                            ifStackIndex--;
                        ifLevel--;
                    }

                    if (expecting_do_then) {
                        ReportTokenError(token, "missing 'do' after last 'if'");
                        otherErrors = true;
                    }
                }
                else if (IsType(token, "endon")) {
                    if (onLevel == 0) {
                        ReportTokenError(token, "'endon' without matching 'on'");
                        otherErrors = true;
                    } else
                        onLevel--;

                    if (expecting_do_then) {
                        ReportTokenError(token, "missing 'do' after last 'on'");
                        otherErrors = true;
                    }
                } else if (onLevel == 0 && ifLevel == 0) {
                    ReportTokenError(token, "tokens cannot be outside root blocks");
                    otherErrors = true;
                }
            }

            if (ifLevel != 0) {
                for (int i=0;i<ifStackIndex;i++) { // only print last 'errors'
                    ReportTokenError(*ifStack[i], "Unmatched 'if' block");
                }
                
            }
            if (onLevel != 0) {
                if (lastOn)
                    ReportTokenError(*lastOn, "Unmatched 'on' block: ");
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
                    for (int j=i;j<i+conditionTokenCount;j++) {
                        if (tokens[j].EqualsIC("and")) {
                            char* str = (char*)tokens[j].start; // need to change this text
                            str[0] = '&';
                            str[1] = '&';
                            str[2] = '\0';
                            tokens[j].end--;
                        } else if (tokens[j].EqualsIC("or")) {
                            char* str = (char*)tokens[j].start; // need to change this text
                            str[0] = '|';
                            str[1] = '|';
                        }
                    }
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
                        tokens[j].isAction = true;
                        j += lineTokenCount;
                    } else {
                        tokens[j].isAction = true;
                        j++; // single token, skip merging
                    }
                }
                i = end - 1; // continue after this block
            }
            return true;
        }

        bool Parser::MergeActions2(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            for (int i = 0; i < tokenCount; ++i) {
                if (!tokens[i].EqualsICAny(actionStartKeywords)) {
                    continue;
                }

                int start = i + 1;
                int end = -1;

                for (int j = start; j < tokenCount; ++j) {
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

                    // Count tokens for current logical action line, respecting HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD continuation
                    /*while (j + lineTokenCount < end &&
                        (tokens[j + lineTokenCount].line == currentLine || 
                            // If previous line ends with HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD, continue to next line's tokens
                            (lineTokenCount > 0 &&
                            tokens[j + lineTokenCount - 1].Equals(HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD)))) {
                        lineTokenCount++;
                    }*/

                    while (j + lineTokenCount < end) {
                        // If token line is current line, include it
                        if (tokens[j + lineTokenCount].line == currentLine) {
                            lineTokenCount++;
                        }
                        // If last token on current line is HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD, include next line tokens
                        else if (lineTokenCount > 0 && tokens[j + lineTokenCount - 1].Equals(HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD)) {
                            // extend currentLine to next line to continue merging
                            currentLine = tokens[j + lineTokenCount].line;
                            lineTokenCount++;
                        } else {
                            break;
                        }
                    }

                    if (lineTokenCount > 1) {
                        tokens[j].InitSubTokens(lineTokenCount);
                        tokens[j].isAction = true;
                        j += lineTokenCount;
                    } else {
                        tokens[j].isAction = true;
                        j++;
                    }
                }
                i = end - 1;
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
                        const Token& innerToken = tokens[j];
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
                        const Token& innerToken = tokens[j];
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
                        const Token& innerToken = tokens[j];
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
            const Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                const Token& token = tokens[i];
                if ((IsType(token, "then") || IsType(token, "else")) == false) continue;
                if (token.itemsInBlock == 0) {
                    ReportTokenError(token, "EnsureActionBlocksContainItems - empty action(s) block detected");
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
                const Token& token = tokens[i];
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
                
                if (Expressions::ValidateExpression(conditions, ExpressionContext::IfCondition) == false) anyError = true;

            }
            return anyError == false;
        }

        bool Parser::VerifyActionBlocks(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                Token& token = tokens[i];
                // 'isAction' is set only on the root token of an action (whether single-token or merged multi-token).
                // Subtokens inside merged tokens do not have 'isAction' set.
                if (token.isAction == false) continue;

                Tokens expressionTokens;
                expressionTokens.items = &token;
                if (token.subTokenCount != 0) {
                    expressionTokens.count = token.subTokenCount;
                } else {
                    expressionTokens.count = 1;
                }
                // first check if there is any currently unsupported AssignKeywords
                const char* match = nullptr;
                //const char* matchKeyword = nullptr;
                //int firstAssignmentOperatorTokenIndex = -1;
                Token* firstAssignmentOperatorToken = nullptr;
                const char* firstAssignmentOperator = nullptr;
                const char* firstCompoundAssignmentOperator = nullptr;
                bool foundAdditionalAssignmentOperators = false;
                
                for (int j = 0; j < expressionTokens.count; ++j) {
                    Token& exprToken = expressionTokens.items[j];
                    const char* searchStart = exprToken.start;
                    //std::cout << "searching:" << token.ToString() << "\n";
                    do {
                        match = exprToken.FindChar('=', searchStart);
                        if (match) {
                            if (firstAssignmentOperator) {
                                foundAdditionalAssignmentOperators = true;
                                anyError = true;
                                Token reportToken;
                                reportToken.line = exprToken.line;
                                reportToken.column = exprToken.column + (match - exprToken.start);
                                ReportTokenError(reportToken, "Found additional assignment keyword");
                                
                            } else {
                                firstAssignmentOperator = match;
                                firstAssignmentOperatorToken = &exprToken;
                                //firstAssignmentOperatorTokenIndex = j;
                                const char* prevChar = match-1;
                                if (exprToken.ContainsPtr(prevChar) && Expressions::IsSingleOperator(*prevChar)) {
                                    // this mean that we found a Compound Assignment Operator
                                    firstCompoundAssignmentOperator = prevChar;
                                }
                            }
                        
                            
                            // Advance search start
                            searchStart = match + 1;

                            if (searchStart >= exprToken.end) break;
                        }
                    
                    } while (match);
                }
                // have:
                // const char* firstAssigmentOperator // is set when a assigment operator is found
                // const char* firstCompundAssignmentOperator // is set when a compund assigment operator is found
                // bool foundAdditionalAssigmentOperators
                
                if (foundAdditionalAssignmentOperators) {
                    continue; // skip for now as it would be hard to extract anything from such string
                }
                if (firstAssignmentOperator == nullptr) {
                    // invalid action line if no assigmend operator is found
                    // maybe in future i can support direct function calls like: somefunc(var2)
                    ReportTokenError(token, "Did not find any assignment keyword");
                    anyError = true;
                    continue; 
                }
                const char* firstAssigmentOperatorStart = nullptr;

                if (firstCompoundAssignmentOperator) {
                    char ch = *firstCompoundAssignmentOperator;
                    
                    if (ch == '<' || ch == '>') {
                        const char* prevChar = firstCompoundAssignmentOperator - 1;
                        // Handle shift compound assignment (<<= or >>=)
                        bool prevIsValid = firstAssignmentOperatorToken->ContainsPtr(prevChar) &&
                                           (*prevChar == '<' || *prevChar == '>');
                        
                        if (prevIsValid) {
                            firstAssigmentOperatorStart = prevChar; // valid <<= or >>=
                        } else {
                            anyError = true;
                            ReportTokenError(token, "missing additional < or > in compound shift assignment keyword");
                            firstAssigmentOperatorStart = firstCompoundAssignmentOperator;
                        }
                    } else {
                        // Not a shift op, treat as normal compound assignment like +=, -=
                        firstAssigmentOperatorStart = firstCompoundAssignmentOperator;
                    }
                }
                Token zcLHS_AssignmentOperand;
                Tokens zcRHS_AssignmentOperands;
                zcLHS_AssignmentOperand.start = token.start;
                zcLHS_AssignmentOperand.line = token.line;
                zcLHS_AssignmentOperand.column = token.column;


                if (token.start == firstAssignmentOperatorToken->start) {
                    // this mean that the assigmentOperator is in the first token
                    // someVar= 5 or someVar=5(if this then token.subTokenCount == 0)
                    if (token.subTokenCount == 0) {
                        zcRHS_AssignmentOperands.items = expressionTokens.items;
                        zcRHS_AssignmentOperands.count = 1;
                        zcRHS_AssignmentOperands.firstTokenStartOffset = firstAssignmentOperator + 1;
                    } else {
                        zcRHS_AssignmentOperands.items = expressionTokens.items+1;
                        zcRHS_AssignmentOperands.count = expressionTokens.count-1;
                    }
                    zcLHS_AssignmentOperand.end = firstAssigmentOperatorStart;
                } else {
                    // this mean that the assigmentOperator is
                    // separated from the first operand

                    // someVar =5 or someVar +=5
                    if (firstAssignmentOperatorToken->ContainsPtr(firstAssignmentOperator+1)) {
                        //std::cout << firstAssignmentOperatorToken->ToString() << " -> ContainsPtr("<<firstAssignmentOperator+1<<"): " << "\n";
                        zcRHS_AssignmentOperands.items = firstAssignmentOperatorToken;
                        zcRHS_AssignmentOperands.count = expressionTokens.count-1;
                        zcRHS_AssignmentOperands.firstTokenStartOffset = firstAssignmentOperator + 1;
                        ZeroCopyString zcTemp(firstAssignmentOperator + 1, firstAssignmentOperator + 2);
                        std::cout << zcTemp.ToString() << "++++++++++++++++++++++++++++++++++++++++++++ zcRHS_AssignmentOperands: " << zcRHS_AssignmentOperands.ToString() << "\n";
                    }
                    // someVar = 6 or someVar += 5 
                    else {
                        zcRHS_AssignmentOperands.items = firstAssignmentOperatorToken+1;
                        zcRHS_AssignmentOperands.count = expressionTokens.count-2;
                    }
                    zcLHS_AssignmentOperand.end = token.end;
                }
                //ReportTokenInfo(zcLHS_AssignmentOperand,  "found zcLHS_AssignmentOperand: ", zcLHS_AssignmentOperand.ToString().c_str());
                
                //ReportInfo("\nzcRHS_AssignmentOperands:");
                //ReportInfo(PrintTokens(zcRHS_AssignmentOperands));
                //ReportInfo("\n");
                std::cout << "***************************************************** zcRHS_AssignmentOperands: " << zcRHS_AssignmentOperands.ToString() << "\n";
                // TODO
                // need to check if we can safely split out a leftside var
                // and then we should first make sure that it allows write
                // could also support cmd like targets like <path>#<cmd>
                // but that's just a matter of splitting by # as such <path>#<cmd> should never contain any spaces
                // and then we can take out the right side and use ValidateExpression
                // on that

                // use the following to validate the right side of the expression
                if (Expressions::ValidateExpression(zcRHS_AssignmentOperands, ExpressionContext::Assignment) == false) anyError = true;

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
            MergeActions2(_tokens);
            ReportInfo("[OK]\n");
            
            ReportInfo("\nCountBlockItems: ");
            CountBlockItems(_tokens); // sets the metadata itemsInBlock
            ReportInfo("[OK]\n");
            
            ReportInfo("\nEnsureActionBlocksContainItems: ");
            if (EnsureActionBlocksContainItems(_tokens) == false) { // uses the metadata itemsInBlock to determine if there are invalid
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

            ReportInfo("\nVerifyConditionBlocks: \n");
            if (VerifyConditionBlocks(_tokens) == false) {
                ReportInfo("[FAIL @ VerifyConditionBlocks]\n");
                return false;
            }

            ReportInfo("\nVerifyActionBlocks: \n");
            if (VerifyActionBlocks(_tokens) == false) {
                ReportInfo("[FAIL @ VerifyActionBlocks]\n");
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