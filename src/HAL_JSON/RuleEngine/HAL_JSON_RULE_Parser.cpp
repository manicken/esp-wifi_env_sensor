
#include "HAL_JSON_RULE_Parser.h"

namespace HAL_JSON {
    namespace Rules {
        static const char* actionStartKeywords[] = {";", "then", "do", "and", "else", "endif", nullptr};
        static const char* actionEndKeywords[] = {";", "and", "if", "else", "elseif", "endon", "endif", nullptr};

        static const TokenType actionStartTypes[] = {TokenType::ActionSeparator, TokenType::And, TokenType::Else, TokenType::EndIf, TokenType::Then, TokenType::NotSet};
        static const TokenType actionEndTypes[] =   {TokenType::ActionSeparator, TokenType::And, TokenType::Else, TokenType::EndIf, TokenType::If, TokenType::ElseIf, TokenType::EndOn, TokenType::NotSet};
        //static const char* actionUnsupportedAssignKeywords[] {"+=", "-=", "*=", "/=", "%=", "&=", "|=", nullptr};
       // static const char* actionAssignKeywords[] {"=", nullptr};

        
        
        void Parser::ReportError(const char* msg) {
    #ifdef _WIN32
            std::cout << "Error: " << msg << std::endl;
    #else
            GlobalLogger.Error(F("Rule Set Parse:"), msg);
    #endif
        }
#ifdef _WIN32
        void Parser::ReportInfo(std::string msg) {
            std::cout << msg;
        }
#else
#define ReportInfo(msg)        
#endif
        
        


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
                 //if (token_start==nullptr)
                 //   std::cout << "token_start is nullptr\n";
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
                Token& token = tokens[i];
                if (token.type == TokenType::If) {
                    currLevel++;
                    if (currLevel > maxLevel) maxLevel = currLevel;
                }
                else if (token.type == TokenType::EndIf) {
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
            int last_If_or_On_Index = 0;

            for (int i = 0; i < tokenCount; i++) {
                Token& token = tokens[i];
                if (token.type == TokenType::If) {
                    ifLevel++;
                    ifStack[ifStackIndex++] = &token;
                    expecting_do_then = true;
                    last_If_or_On_Index = i;
                }
                else if (token.type == TokenType::ElseIf) {
                    expecting_do_then = true;
                    last_If_or_On_Index = i;

                    Token* currentIf = ifStack[ifStackIndex - 1]; // top of the stack

                    // check that no Else has already been found
                    if (currentIf->hasElse) {
                        ReportTokenError(token, "'elseif' cannot appear after 'else'");
                        otherErrors = true;
                    }
                }
                else if (token.type == TokenType::Then) {
                    if (!expecting_do_then) {
                        ReportTokenError(token, "'do/then' without preceding 'if' or 'on'");
                        otherErrors = true;
                    }
                    else if (last_If_or_On_Index + 1 == i) {
                        ReportTokenError(token, "Missing condition between 'if/on/elseif' and 'then/do'");
                        otherErrors = true;
                    }
                    expecting_do_then = false;
                }
                else if (token.type == TokenType::EndIf) {
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
                        ReportTokenError(token, "missing 'do/then' after last 'if'");
                        otherErrors = true;
                    }
                }
                else if (token.type == TokenType::Else) {
                    Token* currentIf = ifStack[ifStackIndex - 1]; // top of the stack

                    // check for multiple Else
                    if (currentIf->hasElse) {
                        ReportTokenError(token, "Multiple 'else' blocks in the same 'if'");
                        otherErrors = true;
                    } else {
                        currentIf->hasElse = 1;
                    }
                } 
                else if (token.type == TokenType::On) {
                    if (ifLevel != 0 || onLevel != 0) {
                        ReportTokenError(token, "'on' block cannot be nested");
                        otherErrors = true;
                    } else {
                        lastOn = &token;
                        onLevel++;
                        expecting_do_then = true;
                        last_If_or_On_Index = i;
                    }
                }
                else if (token.type == TokenType::EndOn) {
                    if (onLevel == 0) {
                        ReportTokenError(token, "'endon' without matching 'on'");
                        otherErrors = true;
                    } else
                        onLevel--;

                    if (expecting_do_then) {
                        ReportTokenError(token, "missing 'do' after last 'on'");
                        otherErrors = true;
                    }
                }
                else if (onLevel == 0 && ifLevel == 0) {
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
                if (tokens[i].type == TokenType::Then) {
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
                if (((tokens[i].type == TokenType::If) || (tokens[i].type == TokenType::ElseIf)) == false) continue;
                i++;
                int conditionTokenCount = CountConditionTokens(_tokens, i);
#ifdef _WIN32   
                std::cout << "If case token count: " << conditionTokenCount << "\n";
#endif
                if (conditionTokenCount == -1) return false; // failsafe
                
                // if multiple tokens then there could be 'and' / 'or' keywords
                // replace them with && || respective
                // for uniform and easier condition parse
                if (conditionTokenCount > 1) {
                    
                    for (int j=i;j<i+conditionTokenCount;j++) {
                        if (tokens[j].type == TokenType::And) {
                            char* str = (char*)tokens[j].start; // need to change this text
                            str[0] = '&';
                            str[1] = '&';
                            str[2] = '\0';
                            tokens[j].end--;
                        } else if (tokens[j].type == TokenType::Or) {
                            char* str = (char*)tokens[j].start; // need to change this text
                            str[0] = '|';
                            str[1] = '|';
                        }
                    }
                    
                }

                tokens[i].MarkTokenGroup(conditionTokenCount, TokenType::IfCondition);
                i += conditionTokenCount; // skip all
            }
            return true;
        }

        bool Parser::MergeActions2(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int level = 0;
            for (int i = 0; i < tokenCount; ++i) {
                if (!tokens[i].AnyType(actionStartTypes)) { //.EqualsICAny(actionStartKeywords)) {
                    if (tokens[i].type == TokenType::If) level++;
                    continue;
                }
                if (tokens[i].type == TokenType::And) tokens[i].type = TokenType::Ignore;
                else if (tokens[i].type == TokenType::ActionSeparator) tokens[i].type = TokenType::Ignore;
                if (tokens[i].type == TokenType::EndIf) { level--;
                    if (level == 0) {
                        ReportTokenInfo(tokens[i], "iflevel is zero");
                        continue; // skip start endif if at root level
                    }
                }

                int start = i + 1;
                int end = -1;

                for (int j = start; j < tokenCount; ++j) {
                    if (tokens[j].AnyType(actionEndTypes)) { //.EqualsICAny(actionEndKeywords)) {
                        
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
                    while (j + lineTokenCount < end) {
                        // If token line is current line, include it
                        if (tokens[j + lineTokenCount].line == currentLine) {
                            lineTokenCount++;
                        }
                        // If last token on current line is HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD, include next line tokens
                        else if (lineTokenCount > 0 && tokens[j + lineTokenCount - 1].type == TokenType::ActionJoiner) { // .Equals(HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD)) {
                            // extend currentLine to next line to continue merging
                            currentLine = tokens[j + lineTokenCount].line;
                            lineTokenCount++;
                        } else {
                            break;
                        }
                    }
                    tokens[j].MarkTokenGroup(lineTokenCount, TokenType::Action);
                    j += lineTokenCount;
                }
                i = end - 1;
            }
            return true;
        }
// this version do work but is ineffective
        void Parser::CountBlockItems(Tokens& _tokens) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int rootLevelBlockCount = 0;
            int blockLevel = 0;

            for (int i = 0; i < tokenCount; ++i) {
                Token& token = tokens[i];

                // Track root-level blocks
                if (token.type == TokenType::If || token.type == TokenType::On) {
                    if (blockLevel == 0)
                        rootLevelBlockCount++;
                    blockLevel++;
                }
                else if (token.type == TokenType::EndIf || token.type == TokenType::EndOn) {
                    blockLevel--;
                }

                // Count branches in an If block: Then + ElseIf + optional Else
                if (token.type == TokenType::If) {
                    int branchCount = 1;  // initial Then branch
                    token.hasElse = 0;

                    for (int j = i + 1; j < tokenCount; ++j) {
                        Token& t = tokens[j];
                        if (t.MergedOrIgnore()) continue;

                        // Break at parent EndIf
                        if (t.type == TokenType::EndIf) break;

                        // Count ElseIf or Else
                        if (t.type == TokenType::ElseIf || t.type == TokenType::Else) {
                            branchCount++;
                            if (t.type == TokenType::Else) {
                                token.hasElse = 1;
                            }
                        }

                        // Skip nested If blocks entirely
                        if (t.type == TokenType::If) {
                            int nestedLevel = 1;
                            for (++j; j < tokenCount && nestedLevel > 0; ++j) {
                                Token& nt = tokens[j];
                                if (nt.MergedOrIgnore()) continue;
                                if (nt.type == TokenType::If) nestedLevel++;
                                else if (nt.type == TokenType::EndIf) nestedLevel--;
                            }
                            --j; // adjust outer loop
                        }
                    }

                    token.itemsInBlock = branchCount;
                }
                // Count actions in Then, ElseIf, Else, or On blocks
                else if (token.type == TokenType::Then || token.type == TokenType::Else || 
                        token.type == TokenType::ElseIf || token.type == TokenType::On) {

                    int count = 0;
                    int nestedLevel = 0;
                    TokenType endTokenType = (token.type == TokenType::On) ? TokenType::EndOn : TokenType::EndIf;

                    for (int j = i + 1; j < tokenCount; ++j) {
                        Token& t = tokens[j];
                        if (t.MergedOrIgnore()) continue;
                        if (t.type == TokenType::And || t.type == TokenType::ActionSeparator) continue;

                        // Nested If counts as single statement in parent branch
                        if (t.type == TokenType::If) {
                            nestedLevel++;
                            count++;
                            continue;
                        }

                        // Track nested EndIf/EndOn
                        if (t.type == TokenType::EndIf || t.type == TokenType::EndOn) {
                            if (nestedLevel > 0) {
                                nestedLevel--;
                                continue;
                            } else {
                                break; // branch ends
                            }
                        }

                        // Stop at next sibling branch
                        if (nestedLevel == 0 && (t.type == TokenType::ElseIf || t.type == TokenType::Else)) {
                            break;
                        }

                        if (nestedLevel == 0) count++;
                    }

                    token.itemsInBlock = count;
                }
            }

            _tokens.rootBlockCount = rootLevelBlockCount;
        }
            /*
            // should work but counts wrong when nested if
            void Parser::CountBlockItems(Tokens& _tokens) {
                Token* tokens = _tokens.items;
                int tokenCount = _tokens.count;
                int rootLevelBlockCount = 0;

                int ifTokenCount = Count_IfTokens(_tokens);
                if (ifTokenCount <= 0) ifTokenCount = 1;
                Token** ifStack = new Token*[ifTokenCount]();
                int ifStackIndex = 0;
                int onLevel = 0;

                for (int i = 0; i < tokenCount; i++) {
                    Token& token = tokens[i];

                    // Track root-level blocks
                    if (token.type == TokenType::If) {
                        if (ifStackIndex == 0 && onLevel == 0)
                            rootLevelBlockCount++;

                        ifStack[ifStackIndex++] = &token;
                        token.hasElse = 0;
                        token.itemsInBlock = 1; // Then branch counts as 1

                    } else if (token.type == TokenType::On) {
                        if (ifStackIndex == 0 && onLevel == 0)
                            rootLevelBlockCount++;

                        onLevel++;
                        token.itemsInBlock = 0;
                    }
                    // End of If/On blocks
                    else if (token.type == TokenType::EndIf) {
                        if (ifStackIndex > 0) ifStackIndex--;
                    } else if (token.type == TokenType::EndOn) {
                        if (onLevel > 0) onLevel--;
                    }

                    // Count branches (ElseIf / Else) inside top If
                    if (token.type == TokenType::ElseIf || token.type == TokenType::Else) {
                        if (ifStackIndex > 0) {
                            Token* currentIf = ifStack[ifStackIndex - 1];
                            currentIf->itemsInBlock++;

                            if (token.type == TokenType::Else)
                                currentIf->hasElse = 1;
                        }
                    }

                    // Count statements inside Then/Else/ElseIf/On
                    if (token.type == TokenType::Then || token.type == TokenType::Else || 
                        token.type == TokenType::ElseIf || token.type == TokenType::On) {

                        int count = 0;
                        int nestedLevel = 0;
                        TokenType endTokenType = (token.type == TokenType::On) ? TokenType::EndOn : TokenType::EndIf;

                        for (int j = i + 1; j < tokenCount; ++j) {
                            Token& t = tokens[j];
                            if (t.MergedOrIgnore()) continue;
                            if (t.type == TokenType::And || t.type == TokenType::ActionSeparator) continue;

                            if (t.type == TokenType::If) {
                                nestedLevel++;
                                count++;
                                continue;
                            }
                            if (t.type == TokenType::EndIf || t.type == TokenType::EndOn) {
                                if (nestedLevel > 0) nestedLevel--;
                                else break;
                            }
                            if (nestedLevel == 0 && (t.type == TokenType::ElseIf || t.type == TokenType::Else))
                                break;

                            if (nestedLevel == 0) count++;
                        }

                        token.itemsInBlock = count;
                    }
                }

                _tokens.rootBlockCount = rootLevelBlockCount;
                delete[] ifStack;
            }*/
    /* still dont work chatgpt is really stupid
void Parser::CountBlockItems(Tokens& _tokens) {
    Token* tokens = _tokens.items;
    int tokenCount = _tokens.count;

    // Allocate stack for nested blocks
    int maxBlocks = Count_IfTokens(_tokens);
    if (maxBlocks <= 0) maxBlocks = 1; // at least 1
    Token** blockStack = new Token*[32]();
    int stackIndex = 0;

    int rootLevelBlockCount = 0;

    for (int i = 0; i < tokenCount; i++) {
        Token& token = tokens[i];

        // Skip merged/ignored tokens if needed
        if (token.MergedOrIgnore()) continue;

        switch (token.type) {

            case TokenType::If:
                if (stackIndex == 0) rootLevelBlockCount++;
                token.itemsInBlock = 1; // base Then branch
                token.hasElse = 0;
                blockStack[stackIndex++] = &token;
                break;

            case TokenType::On:
                if (stackIndex == 0) rootLevelBlockCount++;
                token.itemsInBlock = 0;
                blockStack[stackIndex++] = &token;
                break;

            case TokenType::Then:
            case TokenType::ElseIf:
            case TokenType::Else:
                if (stackIndex > 0) {
                    Token* top = blockStack[stackIndex - 1];
                    // Only If blocks can have Else / ElseIf
                    if (token.type == TokenType::Else)
                        top->hasElse = 1;
                    // Count this branch as one statement in the top block
                    if (token.type != TokenType::Then)
                        top->itemsInBlock++;
                }
                break;

            case TokenType::EndIf:
            case TokenType::EndOn:
                if (stackIndex > 0) stackIndex--;
                break;

            default:
                // Regular statement or action
                if (stackIndex > 0) {
                    Token* top = blockStack[stackIndex - 1];
                    top->itemsInBlock++;
                }
                break;
        }
    }

    _tokens.rootBlockCount = rootLevelBlockCount;
    delete[] blockStack;
}
*/

        bool Parser::EnsureActionBlocksContainItems(Tokens& _tokens) {
            const Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                const Token& token = tokens[i];
                if (((token.type == TokenType::Then) || (token.type == TokenType::Else)) == false) continue;
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
                if (((token.type == TokenType::If) || (token.type == TokenType::ElseIf)) == false) continue;
                //const char* conditions = tokens[i+1].text;
                Tokens conditions;
                conditions.items = &tokens[i+1];
                if (tokens[i+1].itemsInBlock != 0) {
                    conditions.count = tokens[i+1].itemsInBlock;
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
                // 'TokenType::Action' is set only on the root token of an action (whether single-token or merged multi-token).
                // Subtokens inside merged tokens do not have the type TokenType::Action, instead they have the type TokenType::Merged
                if (token.type != TokenType::Action) continue;

                Tokens expressionTokens;
                expressionTokens.items = &token;
                if (token.itemsInBlock != 0) {
                    expressionTokens.count = token.itemsInBlock;
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
                } else {
                    firstAssigmentOperatorStart = firstAssignmentOperator;
                }
                Token zcLHS_AssignmentOperand;
                Tokens zcRHS_AssignmentOperands;
                zcLHS_AssignmentOperand.start = token.start;
                zcLHS_AssignmentOperand.line = token.line;
                zcLHS_AssignmentOperand.column = token.column;

                if (token == *firstAssignmentOperatorToken) { 
#ifdef _WIN32
                    std::cout << "(token.start == firstAssignmentOperatorToken->start):" << token.ToString() << "\n";
#endif
                    // this mean that the assigmentOperator is in the first token
                    // someVar= 5 or someVar=5(if this then token.itemsInBlock == 0)
                    if (token.itemsInBlock == 0) {
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
                    }
                    // someVar = 6 or someVar += 5 
                    else {
                        zcRHS_AssignmentOperands.items = firstAssignmentOperatorToken+1;
                        zcRHS_AssignmentOperands.count = expressionTokens.count-2;
                    }
                    zcLHS_AssignmentOperand.end = token.end;
                }
#ifdef _WIN32
                std::cout << "zcLHS_AssignmentOperand: " << zcLHS_AssignmentOperand.ToString() << "\n";
#endif
                if (firstCompoundAssignmentOperator) {
                    Expressions::ValidateOperand(zcLHS_AssignmentOperand, anyError, ValidateOperandMode::ReadWrite);
                }
                else {
                    Expressions::ValidateOperand(zcLHS_AssignmentOperand, anyError, ValidateOperandMode::Write);
                }
                // use the following to validate the right side of the expression
                if (Expressions::ValidateExpression(zcRHS_AssignmentOperands, ExpressionContext::Assignment) == false) anyError = true;

            }
            return anyError == false;
        }
#include <chrono>
        bool Parser::ValidateParseRuleSet(Tokens& _tokens, bool validateOnly) {

            if (validateOnly) {
                ReportInfo("**********************************************************************************\n");
                ReportInfo("*                            RAW TOKEN LIST                                      *\n");
                ReportInfo("**********************************************************************************\n");
                ReportInfo(PrintTokens(_tokens,0) + "\n");

                ReportInfo("\nVerifyBlocks (BetterError): ");
                MEASURE_TIME("VerifyBlocks time: ",
                if (validateOnly && VerifyBlocks(_tokens) == false) {
                    ReportInfo("[FAIL]\n");
                    return false;
                }
                );
                ReportInfo("[OK]\n");
            }
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

            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            PARSED TOKEN LIST                                   *\n");
            ReportInfo("**********************************************************************************\n");

            ReportInfo(PrintTokens(_tokens,0) + "\n");
            
            if (validateOnly) {
                ReportInfo("\nEnsureActionBlocksContainItems: ");
                if (validateOnly && EnsureActionBlocksContainItems(_tokens) == false) { // uses the metadata itemsInBlock to determine if there are invalid
                    ReportInfo("[FAIL]\n");
                    return false;
                }
                ReportInfo("[OK]\n");

                

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
            }
            return true;
        }

        bool Parser::ReadAndParseRuleSetFile(const char* filePath, void (*parsedOKcallback)(Tokens& tokens)) {
            size_t fileSize;
            char* fileContents;// = ReadFileToMutableBuffer(filePath, fileSize);
            LittleFS_ext::FileResult fileResult = LittleFS_ext::load_from_file(filePath, &fileContents, &fileSize);
            if (fileResult != LittleFS_ext::FileResult::Success) {
                ReportInfo("Error: file could not be read/or is empty\n");
                return false;
            }

            MEASURE_TIME("FixNewLines and StripComments time: ",
            // fix newlines so that they only consists of \n 
            // for easier parsing
            FixNewLines(fileContents);
            // replaces all comments with whitespace
            // make it much simpler to parse the contents 
            StripComments(fileContents);
            );

            int tokenCount = CountTokens(fileContents);
            ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
            Tokens tokens(tokenCount);
            
            MEASURE_TIME("Tokenize time: ",

            if (Tokenize(fileContents, tokens) == false) {
                ReportInfo("Error: could not Tokenize\n");
                delete[] fileContents;
                return false;
            }
            );

            bool anyError = false;
            if (ValidateParseRuleSet(tokens, parsedOKcallback==nullptr)) {
                ReportInfo("ParseRuleSet [OK]\n");

                if (parsedOKcallback)
                    parsedOKcallback(tokens);

            } else {
                ReportInfo("ParseRuleSet [FAIL]\n");
                anyError = true;
            }
            // dont forget to free/delete
            delete[] fileContents;
            return anyError == false;
        }

        bool Parser::ParseExpressionTest(const char* filePath) {
            size_t fileSize;
            char* fileContents;// = ReadFileToMutableBuffer(filePath, fileSize);
            LittleFS_ext::FileResult fileResult = LittleFS_ext::load_from_file(filePath, &fileContents, &fileSize);
            if (fileResult != LittleFS_ext::FileResult::Success) {
                ReportInfo("Error: file could not be read/or is empty\n");
                return false;
            }

            MEASURE_TIME("FixNewLines and StripComments time: ",
            // fix newlines so that they only consists of \n 
            // for easier parsing
            FixNewLines(fileContents);
            // replaces all comments with whitespace
            // make it much simpler to parse the contents 
            StripComments(fileContents);
            );

            int tokenCount = CountTokens(fileContents);
            ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
            Tokens tokens(tokenCount);
            
            MEASURE_TIME("Tokenize time: ",

            if (Tokenize(fileContents, tokens) == false) {
                ReportInfo("Error: could not Tokenize\n");
                delete[] fileContents;
                return false;
            }
            );

            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            PARSED TOKEN LIST                                   *\n");
            ReportInfo("**********************************************************************************\n");

            ReportInfo(PrintTokens(tokens,0) + "\n");

            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            VALIDATE PARSED TOKEN LIST                          *\n");
            ReportInfo("**********************************************************************************\n");
            if (Expressions::ValidateExpression(tokens, ExpressionContext::IfCondition) == false)
            {
                ReportInfo("Error: validate tokens fail\n");
                delete[] fileContents;
                return false;
            }

            ExpressionTokens* expressionTokens = Expressions::preParseTokens(tokens);
            if (expressionTokens == nullptr) {
                ReportInfo("Error: could not pre parse tokens\n");
                delete[] fileContents;
                return false;
            }


            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            EXPRESSION TOKEN LIST  raw                          *\n");
            ReportInfo("**********************************************************************************\n");

            ReportInfo(PrintExpressionTokens(*expressionTokens) + "\n");

            
            

            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            EXPRESSION TOKEN LIST  PARSING                      *\n");
            ReportInfo("**********************************************************************************\n");
            //ReportInfo("expressionTokens->count:" + std::to_string(expressionTokens->count) + "\n");
            int opStackSize = expressionTokens->count;
            int outStackSize = expressionTokens->count;
            int tempStackSize = expressionTokens->count;

            ParseContext parseContext(opStackSize, outStackSize, tempStackSize);
            LogicRPNNode* lrpnNode = Expressions::ParseConditionalExpression(*expressionTokens, parseContext);

            if (lrpnNode == nullptr) {
                ReportInfo("Error: could not ParseConditionalExpression\n");
                delete expressionTokens;
                delete[] fileContents;
                return false;
            }
            Expressions::DoAllInplaceCalcRPN(lrpnNode);
            ReportInfo("linear view:\n");
            Expressions::printLogicRPNNode(lrpnNode);
            ReportInfo("\n\ntree view:\n");
            Expressions::printLogicRPNNodeTree(lrpnNode, 0);

            delete lrpnNode; // deletes the whole tree recursive
            delete expressionTokens;
            delete[] fileContents;

            ReportInfo("\nAll done!!!\n");
            return true;
        }
    }
}