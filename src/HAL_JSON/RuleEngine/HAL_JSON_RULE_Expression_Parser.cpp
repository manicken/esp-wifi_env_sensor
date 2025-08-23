
#include "HAL_JSON_RULE_Expression_Parser.h"



namespace HAL_JSON {
    namespace Rules {
        void SliceStackReportError(const char* msg, const char* param) {
#ifdef _WIN32
            std::cout << "SliceStackError: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Error(F("SliceStack:"), msg);
#endif
        }

        void Expressions::ReportError(const char* msg, const char* param) {
#ifdef _WIN32
            std::cout << "Error: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Error(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportWarning(const char* msg, const char* param) {
#ifdef _WIN32
            std::cout << "Warning: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Warn(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportInfo(std::string msg) {
#ifdef _WIN32
            std::cout << msg;
#else
            //GlobalLogger.Info(F("Expr Rule Parse:"), msg);
#endif
        }

        const char* Expressions::SingleOperatorList = HAL_JSON_RULES_EXPRESSIONS_SINGLE_OPERATOR_LIST;
        const char* Expressions::DoubleOperatorList = HAL_JSON_RULES_EXPRESSIONS_DOUBLE_OPERATOR_LIST;

        bool Expressions::IsSingleOperator(char c) {
            const char* op = SingleOperatorList;
            while (*op != '\0') {
                if (*op == c) return true;
                ++op;
            }
            return false;
        }

        bool Expressions::IsDoubleOperator(const char* c) {
            if (c == nullptr) {
                ReportError("IsDoubleOperator c - was nullptr");
                return false;
            } else if (*c == '\n') {
                ReportError("IsDoubleOperator *c - was empty str");
                return false;
            } else if (*(c+1) == '\n') {
                ReportError("IsDoubleOperator *(c+1) - was empty str");
                return false;
            }
            //if (c == nullptr /*|| *c != '\0' */|| *(c + 1) == '\0') return false; // safety

            const char* op = DoubleOperatorList;
            while (*op != '\0') {
                if ((*op == *c) && (*(op+1) == *(c+1))) return true;
                op+=2;
            }
            return false;
        }

        bool Expressions::IsValidOperandChar(char c) {
            return 
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || 
                c == ':' || 
                c == '.' || 
                c == ',' || 
                c == '#';
        }

        bool Expressions::CountOperatorsAndOperands(Tokens& tokens, int& operatorCount, int& operandCount, int& leftParenthesisCount, ExpressionContext exprContext) {
            operatorCount = 0;
            operandCount = 0;
            leftParenthesisCount  = 0;
            // early checks
            if (tokens.count == 0) {
                ReportError("expr. is empty");
                return false;
            }
            /*if (strlen(expr) == 0) {
                ReportError("expr. is empty");
                return false;
            }*/

            const char* firstTokenStart = nullptr;
            if (tokens.firstTokenStartOffset != nullptr)
                firstTokenStart = tokens.firstTokenStartOffset;
            else
                firstTokenStart = tokens.items[0].start;

            if(IsDoubleOperator(firstTokenStart) && exprContext == ExpressionContext::IfCondition) { // this only checks the two first characters in the Expression
                ReportError("expr. cannot start with a operator");
                return false;
            }
            if(IsSingleOperator(*firstTokenStart)) {
                ReportError("expr. cannot start with a operator");
                return false;
            }

            int rightParenthesisCount = 0;

            bool inOperand = false;

            for (int cti=0;cti<tokens.count;cti++) { // cti = currTokenIndex
                Token& token = tokens.items[cti];
                const char* effectiveStart = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    effectiveStart  = tokens.firstTokenStartOffset;
                    //std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }
                for (const char* p = effectiveStart; p < token.end; p++) {
                    if (IsDoubleOperator(p) && exprContext == ExpressionContext::IfCondition) {
                        p++;
                        operatorCount++;
                        inOperand = false;
                    }
                    else if (IsSingleOperator(*p)) {
                        operatorCount++;
                        inOperand = false;
                    } else if (*p == '(') {
                        leftParenthesisCount++;
                        inOperand = false;
                    } else if (*p == ')') {
                        rightParenthesisCount++;
                        inOperand = false;
                    } else if (!inOperand) {
                        operandCount++;
                        inOperand = true;
                    }
                }
            }
            bool anyError = false;
            if (operatorCount >= operandCount) {
                ReportError("double operator(s) detected");
                anyError = true;
            } else if (operatorCount != operandCount - 1) {
                ReportError("operator(s) missing before/after parenthesis");
                anyError = true;
            }
            if (leftParenthesisCount != rightParenthesisCount) {
                ReportError("mismatch parenthesis detected");
                anyError = true;
            }

            return anyError == false;
        }
    
/* in favor of ZeroCopyString ValidNumber function
        bool Expressions::OperandIsVariable(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (isdigit(*p) == false) return true;
                p++;
            }
            return false;
        }
*/
        const char* Expressions::ValidOperandVariableName(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (IsValidOperandChar(*p) == false) {
                    return p;
                }
                p++;
            }
            return nullptr;
        }

        bool Expressions::ValidateExpression(Tokens& tokens, ExpressionContext exprContext) {
            int operatorCount, operandCount, leftParenthesisCount;
            bool anyError = false;

            anyError = !CountOperatorsAndOperands(tokens, operatorCount, operandCount, leftParenthesisCount, exprContext);

            /*ReportInfo("operatorCount:" + std::to_string(operatorCount) +
                    ", operandCount:" + std::to_string(operandCount) +
                    ", leftParenthesisCount:" + std::to_string(leftParenthesisCount));
*/
            if (anyError) return false;

            int operandIndex = 0;
            bool inOperand = false;
            const char* p = nullptr;
            const char* operandStart = nullptr;
            const char* operandEnd = nullptr;

            for (int cti = 0; cti < tokens.count; ++cti) {
                Token& token = tokens.items[cti];
                const char* effectiveStart  = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    effectiveStart  = tokens.firstTokenStartOffset;
                    //std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }
                for (p = effectiveStart ; p < token.end; ++p) {
                    if (IsDoubleOperator(p)&& exprContext == ExpressionContext::IfCondition) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-effectiveStart);
                            ValidateOperand(operand, anyError);
                            ++operandIndex;
                            inOperand = false;
                        }
                        ++p; // Skip second char of double op
                    } else if (IsSingleOperator(*p) || *p == '(' || *p == ')' || *p == HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD[0]) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-effectiveStart);
                            ValidateOperand(operand, anyError);
                            ++operandIndex;
                            inOperand = false;
                        }
                    } else if (!inOperand) {
                        operandStart = p;
                        inOperand = true;
                    }
                }
                if (inOperand) { // this was outside of the last loop
                    operandEnd = p;
                    Token operand(operandStart, operandEnd);
                    operand.line = token.line;
                    operand.column = token.column + (operandStart-effectiveStart);
                    ValidateOperand(operand, anyError);
                    ++operandIndex;
                    inOperand = false;
                }
            }
            return !anyError;
        }

        void Expressions::ValidateOperand(const Token& operandToken, bool& anyError, ValidateOperandMode mode) {
            //bool operandIsVariable = OperandIsVariable(operandToken);
#ifdef HAL_JSON_RULES_EXPRESSIONS_PARSER_SHOW_DEBUG
            std::string msg;
            //if (OperandIsVariable(operandToken)) {
            if (operandToken.ValidNumber() == false) {
                msg = "Variable: Name= ";
                ZeroCopyString funcName = operandToken;
                ZeroCopyString base = funcName.SplitOffHead('#');
                msg += base.ToString();
                if (funcName.Length() != 0) {
                    msg += ", funcName= ";
                    msg += funcName.ToString();
                }
            } else {
                msg = "Const Value: ";
                msg += operandToken.ToString();
            }
            ReportTokenInfo(operandToken, msg.c_str());
#endif
            // return early if not a operand variable
            //if (OperandIsVariable(operandToken) == false)
            if (operandToken.ValidNumber()) return;
                return;

            const char* currChar = ValidOperandVariableName(operandToken);
            if (currChar /*&& *currChar != '\0'*/) {
                std::string msg = "found invalid character <" + std::to_string(*currChar) +
                                "> in operand: " + operandToken.ToString();
                ReportTokenWarning(operandToken, msg.c_str());
                // continue validation but note the error
            }

            ZeroCopyString varOperand = operandToken;
            ZeroCopyString funcName = varOperand;
            varOperand = funcName.SplitOffHead('#');

            if (varOperand.Length() > HAL_UID::Size) {
                std::string msg = varOperand.ToString() + " length > " + std::to_string(HAL_UID::Size);
                
                ReportTokenError(operandToken, "Operand name too long: ", msg.c_str());
                anyError = true;
            }

            UIDPath path(varOperand);
            Device* device = Manager::findDevice(path);
            if (!device) {
                std::string deviceName = varOperand.ToString();
                ReportTokenError(operandToken, "could not find the device: ", deviceName.c_str());
                anyError = true;
                return;
            }
            if (mode == ValidateOperandMode::Read || mode == ValidateOperandMode::ReadWrite) {
                HALOperationResult readResult = HALOperationResult::UnsupportedOperation;
                if (funcName.Length() != 0) {
                    HALValue halValue;
                    HALReadValueByCmd readValueByCmd(halValue, funcName);
                    readResult = device->read(readValueByCmd);
                    if (readResult != HALOperationResult::Success) {
                        anyError = true;
                        if (readResult == HALOperationResult::UnsupportedCommand) {
                            std::string funcNameStr = ": " + funcName.ToString();
                            ReportTokenError(operandToken, ToString(readResult), funcNameStr.c_str());
                        } else {
                            ReportTokenError(operandToken, ToString(readResult), ": read");
                        }
                        
                    }
                } else {
                    HALValue halValue;
                    readResult = device->read(halValue);
                    if (readResult != HALOperationResult::Success) {
                        ReportTokenError(operandToken, ToString(readResult), ": read");
                        anyError = true;
                    }
                }
            }
            if (mode == ValidateOperandMode::Write || mode == ValidateOperandMode::ReadWrite) {
                HALOperationResult writeResult = HALOperationResult::UnsupportedOperation;
                if (funcName.Length() != 0) {
                    HALValue halValue;
                    HALWriteValueByCmd writeValueByCmd(halValue, funcName);
                    writeResult = device->write(writeValueByCmd);
                    if (writeResult != HALOperationResult::Success) {
                        anyError = true;
                        if (writeResult == HALOperationResult::UnsupportedCommand) {
                            std::string funcNameStr = ": " + funcName.ToString();
                            ReportTokenError(operandToken, ToString(writeResult), funcNameStr.c_str());
                        } else {
                            ReportTokenError(operandToken, ToString(writeResult), ": write");
                        }
                        
                    }
                } else {
                    HALValue halValue;
                    writeResult = device->write(halValue);
                    if (writeResult != HALOperationResult::Success) {
                        ReportTokenError(operandToken, ToString(writeResult), ": write");
                        anyError = true;
                    }
                }
            }
        }

        //    ██████  ██████  ███    ██ 
        //    ██   ██ ██   ██ ████   ██ 
        //    ██████  ██████  ██ ██  ██ 
        //    ██   ██ ██      ██  ██ ██ 
        //    ██   ██ ██      ██   ████ 
        

        #include <cctype>  // for isspace, isdigit, isalpha

        // Count total number of clean tokens first
        int Expressions::preParseTokensCount(const Tokens& rawTokens) {
            int count = 0;
            for (int i = 0; i < rawTokens.count; ++i) {
                const Token& str = rawTokens.items[i];
                int j = 0;
                const int strLength = str.Length();
                while (j < strLength) {
                    char c = str[j];
                    if (c == '(' || c == ')') {
                        ++count; ++j;
                    } else if ((j + 1) < strLength && IsTwoCharOp(c, str[j+1]) != TokenType::NotSet) {
                        ++count; j += 2;
                    } else if (IsSingleOp(c) != TokenType::NotSet) {
                        ++count; ++j;
                    } else {
                        // identifier/number
                        int startIdx = j;
                        while (j < strLength &&
                            str[j] != '(' &&
                            str[j] != ')' &&
                            IsSingleOp(str[j]) == TokenType::NotSet &&
                            !((j + 1) < strLength && IsTwoCharOp(str[j], str[j+1]) != TokenType::NotSet)) {
                            ++j;
                        }
                        ++count;
                    }
                }
            }
            return count;
        }

        // The actual pre-parser
        ExpressionTokens* Expressions::preParseTokens(const Tokens& rawTokens) {
            size_t totalCount = preParseTokensCount(rawTokens);
            ExpressionTokens* cleanTokens = new ExpressionTokens(totalCount);
            ExpressionToken* cleanTokenItems = cleanTokens->items;
            int cleanTokensIndex = 0;
            const int rawTokensCount = rawTokens.count;
            for (int i = 0; i < rawTokensCount; ++i) {
                const Token& str = rawTokens.items[i];

                int j = 0;
                const int strLength = str.Length();
                while (j < strLength) {
                    char c = str[j];

                    // Parentheses
                    if (c == '(' || c == ')') {
                        ExpressionToken& cleanToken = cleanTokenItems[cleanTokensIndex++];
                        //cleanToken.line   = tok.line;
                        //cleanToken.column = tok.column + j;
                        cleanToken.start  = str.start + j;
                        cleanToken.end    = str.start + j + 1;
                        cleanToken.type   = (c == '(')?TokenType::LeftParenthesis:TokenType::RightParenthesis;
                        ++j;
                        continue;
                    }

                    // Two-character operators
                    
                    if ((j + 1) < strLength) {
                        TokenType twoCharOpType = IsTwoCharOp(c, str[j+1]);
                        if (twoCharOpType != TokenType::NotSet) {
                            ExpressionToken& cleanToken = cleanTokenItems[cleanTokensIndex++];
                            //cleanToken.line   = tok.line;
                            //cleanToken.column = tok.column + j;
                            cleanToken.start  = str.start + j;
                            cleanToken.end    = str.start + j + 2;
                            cleanToken.type   = twoCharOpType;
                            j += 2;
                            continue;
                        }
                    }

                    // Single-character operator
                    TokenType oneCharOpType = IsSingleOp(c);
                    if (oneCharOpType != TokenType::NotSet) {
                        ExpressionToken& cleanToken = cleanTokenItems[cleanTokensIndex++];
                        //cleanToken.line   = tok.line;
                        //cleanToken.column = tok.column + j;
                        cleanToken.start  = str.start + j;
                        cleanToken.end    = str.start + j + 1;
                        cleanToken.type   = oneCharOpType;
                        ++j;
                        continue;
                    }

                    // Identifier / number
                    int startIdx = j;
                    while (j < strLength &&
                        str[j] != '(' &&
                        str[j] != ')' &&
                        IsSingleOp(str[j]) == TokenType::NotSet &&
                        !((j + 1) < strLength && IsTwoCharOp(str[j], str[j+1])!=TokenType::NotSet)) {
                        ++j;
                    }
                    ExpressionToken& cleanToken = cleanTokenItems[cleanTokensIndex++];
                    //cleanToken.line   = tok.line;
                    //cleanToken.column = tok.column + startIdx;
                    cleanToken.start  = str.start + startIdx;
                    cleanToken.end    = str.start + j;
                    cleanToken.type   = TokenType::Operand;
                }
            }
            if (cleanTokensIndex != totalCount) {
                std::string msg = "cleanTokensIndex(" + std::to_string(cleanTokensIndex) +  ") != totalCount(" + std::to_string(totalCount) + ")";

                ReportError(msg.c_str());

                throw std::runtime_error(msg); 
            }
            
            return cleanTokens;
        }
/*
        std::vector<ExpressionToken> Expressions::ToCalcRPN(const std::vector<ExpressionToken*>& tokens) {
            std::vector<ExpressionToken> rpn;
            std::stack<ExpressionToken*> opstack;

            for (ExpressionToken* t : tokens) {
                if (t->type == TokenType::Ignore) continue;

                if (IsCalcOperator(t->type)) {
                    while (!opstack.empty() && IsCalcOperator(opstack.top()->type) &&
                        CalcPrecedence(opstack.top()->type) >= CalcPrecedence(t->type)) {
                        rpn.push_back(*opstack.top());
                        opstack.pop();
                    }
                    opstack.push(t);
                } else if (t->type == TokenType::LeftParenthesis) {
                    opstack.push(t);
                } else if (t->type == TokenType::RightParenthesis) {
                    while (!opstack.empty() && opstack.top()->type != TokenType::LeftParenthesis) {
                        rpn.push_back(*opstack.top());
                        opstack.pop();
                    }
                    if (!opstack.empty()) opstack.pop(); // discard "("
                } else {
                    rpn.push_back(*t);
                }
            }
            while (!opstack.empty()) {
                rpn.push_back(*opstack.top());
                opstack.pop();
            }
            return rpn;
        }*/

        void Expressions::InplaceCalcRPN(std::vector<ExpressionToken*>& tokens) {
            std::stack<ExpressionToken*> opstack;
            size_t outIndex = 0; // points to the current output position in tokens

            for (size_t i = 0; i < tokens.size(); ++i) {
                ExpressionToken* t = tokens[i];
                if (t->type == TokenType::Ignore) continue;

                if (IsCalcOperator(t->type)) {
                    while (!opstack.empty() && IsCalcOperator(opstack.top()->type) &&
                        CalcPrecedence(opstack.top()->type) >= CalcPrecedence(t->type)) {
                        tokens[outIndex++] = opstack.top(); // write operator to output
                        opstack.pop();
                    }
                    opstack.push(t);
                } else if (t->type == TokenType::LeftParenthesis) {
                    opstack.push(t);
                } else if (t->type == TokenType::RightParenthesis) {
                    while (!opstack.empty() && opstack.top()->type != TokenType::LeftParenthesis) {
                        tokens[outIndex++] = opstack.top();
                        opstack.pop();
                    }
                    if (!opstack.empty()) opstack.pop(); // discard "("
                } else {
                    tokens[outIndex++] = t; // operands are written in place
                }
            }

            while (!opstack.empty()) {
                tokens[outIndex++] = opstack.top();
                opstack.pop();
            }

            tokens.resize(outIndex); // shrink vector to actual RPN length
        }

        std::string CalcExpressionToString(const std::vector<ExpressionToken*>& calcExpr) {
            std::string out;
            for (int i=0;i<calcExpr.size();i++) {
                out += calcExpr[i]->ToString();
                if (i + 1 < calcExpr.size()) out += " ";
            }
            return out;
        }
        
        // Recursive print
        void Expressions::printLogicRPNNode(const LogicRPNNode* node) {
            if (!node) return;

            if (node->op == nullptr || node->op->IsEmpty()) {
                // Leaf node → print calc RPN
                ReportInfo("[" + CalcExpressionToString(node->calcRPN) + "]");
            } else {
                // Operator node → print children first, then operator
                
                ReportInfo("[");
                if (node->childA) printLogicRPNNode(node->childA); else ReportInfo("null");
                ReportInfo(" ");
                if (node->childB) printLogicRPNNode(node->childB);
                ReportInfo(" " + (node->op?node->op->ToString():""));
                ReportInfo("]");

            }
        }

        void Expressions::printLogicRPNNodeTree(LogicRPNNode* node, int indent) {
            if (!node) return;

            std::string padding(indent * 2, ' '); // 2 spaces per level

            if (node->op == nullptr || node->op->IsEmpty()) {
                // Leaf node → print calc RPN
                ReportInfo(padding + "- calc: [" + CalcExpressionToString(node->calcRPN) + "]\n");

            } else {
                // Operator node

                ReportInfo(padding + "[" + (node->op?TokenTypeToString(node->op->type):"") + "]\n");

                if (node->childA) printLogicRPNNodeTree(node->childA, indent + 1);
                if (node->childB) printLogicRPNNodeTree(node->childB, indent + 1);
             }
        }

        LogicRPNNode::LogicRPNNode()
            : childA(nullptr), childB(nullptr), op(nullptr)/*, type(OpType::Invalid)*/ {}

        LogicRPNNode::~LogicRPNNode() {
            delete childA;
            delete childB;
        }
        bool LogicRPNNode::IsPureCalcNode() {
            return calcRPN.size() != 0;
        }

        ParseContext::ParseContext(int opStackSize, int outStackSize, int tempStackSize): opStack(opStackSize), outStack(outStackSize), tempStack(tempStackSize) { }

        void ParseContext::merge_calc_from(LogicRPNNode* node) {
            std::vector<ExpressionToken*>& calcRPN = node->calcRPN;
            int calcRPNsize = calcRPN.size();
            for (int i=0;i<calcRPNsize;i++) {
                tempStack.push(calcRPN[i]);     
            }
        }
        void ParseContext::FlushTempToNode(LogicRPNNode* node) {
            std::vector<ExpressionToken*>& calcRPN = node->calcRPN;
            calcRPN.clear();
            int tempStackSize = tempStack.size();
            for (int i = 0; i < tempStackSize; ++i) {
                calcRPN.push_back(tempStack[i]);
            }
            tempStack.ClearCurrSlice();
        }
        std::string ParseContext::PrintTempStackSlice() {
            std::string out;
            int size = tempStack.size();
            for (int i = 0; i < size; ++i) {
                out += tempStack[i]->ToString();  // assumes T is a pointer type with ToString()
                if (i + 1 < size) out += " ";
            }
            return out;
        }

        /** used by ParseConditionalExpression */
        void ParseContext::FlushCalc() {
            Expressions::ReportInfo(PrintTempStackSlice() + "\n");

            LogicRPNNode* node = new LogicRPNNode();

            FlushTempToNode(node);
            outStack.push(node);
        }

        void ParseContext::ApplyOperator() {
            
            if (opStack.empty() || outStack.size() < 2) return;
            ExpressionToken* op = opStack.top_n_pop();
            LogicRPNNode* rhs = outStack.top_n_pop();
            LogicRPNNode* lhs = outStack.top_n_pop();

            LogicRPNNode* parent = new LogicRPNNode();
            parent->childA = lhs;
            parent->childB = rhs;
            parent->op = op;
            outStack.push(parent);
        }
        LogicRPNNode* Expressions::ParseConditionalExpression(ExpressionTokens& tokens, ParseContext& ctx) {
            

            //std::cout << "\n********** parsing start:\n";// << PrintExpressionTokens(tokens, start, end);

            int opStackCurrIndex=0, opStackMinIndex=0;
            ctx.opStack.BeginSlice(opStackMinIndex, opStackCurrIndex);
            int outStackCurrIndex=0, outStackMinIndex=0;
            ctx.outStack.BeginSlice(outStackMinIndex, outStackCurrIndex);
            int tempStackCurrIndex=0, tempStackMinIndex=0;
            ctx.tempStack.BeginSlice(tempStackMinIndex, tempStackCurrIndex);
            
            int end = tokens.count;
            for (int i = tokens.index; i < end; i++) {
                //tokens.index = i;
                //std::cout << "\ntokens.index:" << std::to_string(tokens.index) << "\n";
                ExpressionToken& tok = tokens.items[i];
                if (tok.type == TokenType::Ignore) continue;

                if (tok.type == TokenType::LeftParenthesis) {
                    tokens.index = i + 1;
                    LogicRPNNode* sub = ParseConditionalExpression(tokens, ctx);
                    if (sub == nullptr) {
                        ReportError("sub was nullptr\n");
                        return nullptr; // this will propagate down as a error to the caller
                    }

                    // merge into current calc if return is pure calcstream
                    if (sub->IsPureCalcNode()) {
                        ctx.tempStack.push(&tok); // the first parenthesis
                        ctx.merge_calc_from(sub);
                        ctx.tempStack.push(&tokens.items[tokens.index]); // the last parenthesis
                        delete sub; // when it's a pure calc node delete it
                    }else {
                        // flushCalc() here is technically redundant for valid inputs,
                        // because any pending calc tokens should have been flushed
                        // when we saw a logical operator. But keeping it guards against
                        // accidental merging of calc and logic nodes.
                        if (ctx.tempStack.empty() == false) {
                            
                            ReportWarning("found orphaned calc expression:\n");
                            ctx.FlushCalc();
                        }
                        ctx.outStack.push(sub);
                    }                    
                    i = tokens.index; // skip
                }
                else if (tok.type == TokenType::RightParenthesis) {
                    //std::cout << "right found\n";
                    tokens.index = i;
                    break;
                }
                else if (tok.type == TokenType::LogicalAnd || tok.type == TokenType::LogicalOr) {
                    //std::cout << "flush calc because of logic operator\n";
                    if (ctx.tempStack.empty() == false)
                        ctx.FlushCalc();
                    // Applies higher-or-equal precedence operators on the stack before pushing current token
                    while (!ctx.opStack.empty() &&
                        Expressions::LogicPrecedence(ctx.opStack.top()->type) >= Expressions::LogicPrecedence(tok.type)) {
                        ctx.ApplyOperator();
                    }
                    ctx.opStack.push(&tok);
                    tokens.index = i;
                }
                else {
                    //std::cout << "push token to tempStack:" << tok.ToString() << "\n";
                    ctx.tempStack.push(&tok);
                }
                
            }

            //std::cout << "flush leftover calc\n";
            // flush leftover calc
            if (ctx.tempStack.empty() == false)
                ctx.FlushCalc();

            // Apply remaining Operators
            while (!ctx.opStack.empty()) {
                ctx.ApplyOperator();
            }
            //std::cout << "******* parsing end ********\n\n";
            LogicRPNNode* returnValue = ctx.outStack.empty() ? nullptr : ctx.outStack.top();

            // restore slices
            ctx.opStack.Restore(opStackMinIndex, opStackCurrIndex);
            ctx.outStack.Restore(outStackMinIndex, outStackCurrIndex);
            ctx.tempStack.Restore(tempStackMinIndex, tempStackCurrIndex);
            return returnValue;
        }

        void Expressions::DoAllInplaceCalcRPN(LogicRPNNode* node) {
            if (!node) return;

            if (node->op == nullptr || node->op->IsEmpty()) {
                // Leaf node
                InplaceCalcRPN(node->calcRPN);
            } else {
                // Operator node
                if (node->childA) DoAllInplaceCalcRPN(node->childA);
                if (node->childB) DoAllInplaceCalcRPN(node->childB);
             }
        }
        static const TokenType logicOperators[] = {TokenType::LogicalOr, TokenType::LogicalAnd, 
                                                TokenType::NotSet};
        // actually compare and calc operators fall into the same type check
        // but we keep them separate here for clarity
        static const TokenType compareOperators[] = {TokenType::CompareEqualsTo, TokenType::CompareNotEqualsTo,
                                                     TokenType::CompareGreaterThanOrEqual, TokenType::CompareLessThanOrEqual,
                                                     TokenType::CompareGreaterThan, TokenType::CompareLessThan, 
                                                     TokenType::NotSet};
                                                     
        static const TokenType calcOperators[] = {TokenType::CalcPlus, TokenType::CalcMinus, TokenType::CalcMultiply, TokenType::CalcDivide, TokenType::CalcModulus,
                                                  TokenType::CalcBitwiseAnd, TokenType::CalcBitwiseOr, TokenType::CalcBitwiseExOr, 
                                                  TokenType::CalcBitwiseLeftShift, TokenType::CalcBitwiseRightShift, TokenType::NotSet};

        void Expressions::ParseConditionalExpression(ExpressionTokens& tokens) 
        {
            std::vector<ExpressionToken*> opStack;
            std::vector<ExpressionToken*> tempStack;
            std::vector<LogicRPNNode*> outStack;
            opStack.reserve(tokens.count);
            tempStack.reserve(tokens.count);
            outStack.reserve(tokens.count);

            for (int i = 0; i < tokens.count; i++)
            {
                ExpressionToken& token = tokens.items[i];
                if (token.type == TokenType::LeftParenthesis) {
                    opStack.push_back(&token);
                } else if (token.type == TokenType::RightParenthesis) {

                } else if (token.AnyType(calcOperators) || token.AnyType(compareOperators)) {

                } else if (token.AnyType(logicOperators)) {

                } else {
                    tempStack.push_back(&token);
                }
            }
            
        }

    }
}