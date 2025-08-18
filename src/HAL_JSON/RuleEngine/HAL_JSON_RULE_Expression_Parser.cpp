
#include "HAL_JSON_RULE_Expression_Parser.h"

namespace HAL_JSON {
    namespace Rules {
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
            std::cout << msg << std::endl;
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
        
        

        bool Expressions::OperandIsVariable(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (isdigit(*p) == false) return true;
                p++;
            }
            return false;
        }

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
            if (OperandIsVariable(operandToken)) {
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
            if (OperandIsVariable(operandToken) == false)
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
                while (j < str.Length()) {
                    char c = str[j];
                    if (std::isspace(c)) {
                        ++j;
                    } else if (c == '(' || c == ')') {
                        ++count; ++j;
                    } else if (j + 1 < str.Length() && IsTwoCharOp(c, str[j+1]) != TokenType::NotSet) {
                        ++count; j += 2;
                    } else if (IsSingleOp(c) != TokenType::NotSet) {
                        ++count; ++j;
                    } else {
                        // identifier/number
                        while (j < str.Length() && !std::isspace(str[j]) &&
                            str[j] != '(' && str[j] != ')' &&
                            IsSingleOp(str[j]) == TokenType::NotSet) ++j;
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

                    // Skip whitespace
                    if (std::isspace(c)) {
                        ++j;
                        continue;
                    }

                    // Parentheses
                    if (c == '(' || c == ')') {
                        ExpressionToken& cleanToken = cleanTokenItems[cleanTokensIndex++];
                        cleanToken.start  = str.start + j;
                        cleanToken.end    = str.start + j + 1;
                        cleanToken.type   = (c == '(')?TokenType::LeftParenthesis:TokenType::RightParenthesis;
                        ++j;
                        continue;
                    }

                    // Two-character operators
                    TokenType twoCharOpType = IsTwoCharOp(c, str[j+1]);
                    if (j + 1 < strLength && twoCharOpType != TokenType::NotSet) {
                        ExpressionToken& cleanToken = cleanTokenItems[cleanTokensIndex++];
                        //cleanToken.line   = tok.line;
                        //cleanToken.column = tok.column + j;
                        cleanToken.start  = str.start + j;
                        cleanToken.end    = str.start + j + 2;
                        cleanToken.type   = twoCharOpType;
                        j += 2;
                        continue;
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
                        !std::isspace(str[j]) &&
                        str[j] != '(' &&
                        str[j] != ')' &&
                        IsSingleOp(str[j]) == TokenType::NotSet &&
                        !(j + 1 < strLength && IsTwoCharOp(str[j], str[j+1])!=TokenType::NotSet)) {
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

            int parenthesisCount = 0;
            const int cleanTokensCount = cleanTokens->count;
            
            for (int i = 0; i < cleanTokensCount; ++i) {
                if (cleanTokenItems[i].type == TokenType::LeftParenthesis)
                    parenthesisCount++;
            }

            int* parethesisStack = new int[parenthesisCount];
            int parethesisStackIndex = 0;

            for (int i = 0; i < cleanTokensCount; ++i) {
                ExpressionToken& token = cleanTokenItems[i];
                if (token.type == TokenType::LeftParenthesis) {
                    parethesisStack[parethesisStackIndex++] = i;
                } else if (token.type == TokenType::RightParenthesis) {
                    int index = parethesisStack[--parethesisStackIndex];
                    token.matchingIndex = index;
                    cleanTokenItems[index].matchingIndex = i;
                }
            }
            delete[] parethesisStack;

            return cleanTokens;
        }

        void Expressions::MarkRedundantParentheses(ExpressionTokens& tokens) {
            for (size_t i = 0; i < tokens.count; ++i) {
                auto& tok = tokens.items[i];

                // Only care about '('
                if (tok.type != TokenType::LeftParenthesis)
                    continue;

                size_t matching = tok.matchingIndex;
                if (matching == -1 || matching <= i)
                    continue; // safety check

                // Scan inside the parentheses
                bool hasLogical = false;
                int count = 0;
                for (size_t j = i + 1; j < matching; ++j) {
                    if (tokens.items[j].type == TokenType::LeftParenthesis)
                        count++;
                    else if (tokens.items[j].type == TokenType::RightParenthesis)
                        count--;
                    else if (tokens.items[j].type == TokenType::LogicalAnd || tokens.items[j].type == TokenType::LogicalOr)
                        hasLogical = true;
                }

                // If no logical operators inside, these parentheses are redundant
                if (!hasLogical) {
                    tok.type = TokenType::Ignore;               // mark '('
                    tokens.items[matching].type = TokenType::Ignore; // mark ')'
                }
            }
        }

        std::vector<ExpressionToken> Expressions::ToCalcRPN(const std::vector<ExpressionToken>& tokens) {
            std::vector<ExpressionToken> rpn;
            std::stack<ExpressionToken> opstack;

            for (auto& t : tokens) {
                if (t.type == TokenType::Ignore) continue;

                if (IsCalcOperator(t.type)) {
                    while (!opstack.empty() && IsCalcOperator(opstack.top().type) &&
                        CalcPrecedence(opstack.top().type) >= CalcPrecedence(t.type)) {
                        rpn.push_back(opstack.top());
                        opstack.pop();
                    }
                    opstack.push(t);
                } else if (t.type == TokenType::LeftParenthesis) {
                    opstack.push(t);
                } else if (t.type == TokenType::RightParenthesis) {
                    while (!opstack.empty() && opstack.top().type != TokenType::LeftParenthesis) {
                        rpn.push_back(opstack.top());
                        opstack.pop();
                    }
                    if (!opstack.empty()) opstack.pop(); // discard "("
                } else {
                    rpn.push_back(t);
                }
            }
            while (!opstack.empty()) {
                rpn.push_back(opstack.top());
                opstack.pop();
            }
            return rpn;
        }

        std::string CalcExpressionToString(const std::vector<ExpressionToken>& calcExpr) {
            std::string out;
            for (int i=0;i<calcExpr.size();i++) {
                out += calcExpr[i].ToString();
                if (i + 1 < calcExpr.size()) out += " ";
            }
            return out;
        }

        // Recursive print
        void Expressions::printLogicRPNNode(const LogicRPNNode* node) {
            if (!node) return;

            if (node->op == nullptr || node->op->IsEmpty()) {
                // Leaf node → print calc RPN
                //std::cout << " lf(" + std::to_string(node.calcRPN.size()) + ") ";
                std::cout << "[";
                std::cout << CalcExpressionToString(node->calcRPN);
                std::cout << "]";
            } else {
                // Operator node → print children first, then operator
                //std::cout << " op(" + std::to_string(node.children.size()) + ") ";
                std::cout << "[";
                if (node->children[0]) printLogicRPNNode(node->children[0]);
                std::cout << " ";
                if (node->children[1]) printLogicRPNNode(node->children[1]);
                /*for (size_t i = 0; i < 2; ++i) {
                    printLogicRPNNode(node.children[i]);
                    if (i + 1 < 2) std::cout << " ";
                }*/
                std::cout << " " << (node->op?node->op->ToString():"") << "]";
            }
        }

        void Expressions::printLogicRPNNodeTree(const LogicRPNNode* node, int indent) {
            if (!node) return;

            std::string padding(indent * 2, ' '); // 2 spaces per level

            if (node->op == nullptr || node->op->IsEmpty()) {
                // Leaf node → print calc RPN
                std::cout << padding << "- Leaf: [" << CalcExpressionToString(node->calcRPN) << "]\n";
            } else {
                // Operator node
                std::cout << padding << "- Op: " << (node->op?node->op->ToString():"") << "\n";
                if (node->children[0]) printLogicRPNNodeTree(node->children[0], indent + 1);
                if (node->children[1]) printLogicRPNNodeTree(node->children[1], indent + 1);
                /*for (size_t i = 0; i < 2; ++i) {
                    printLogicRPNNodeTree(node.children[i], indent + 1);
                }*/
            }
        }

        bool IsComparisonOperator(TokenType type) {
            switch (type) {
                case TokenType::CompareEqualsTo:
                case TokenType::CompareNotEqualsTo:
                case TokenType::CompareLessThan:
                case TokenType::CompareGreaterThan:
                case TokenType::CompareLessThanOrEqual:
                case TokenType::CompareGreaterThanOrEqual:
                    return true;
                default:
                    return false;
            }
        }

        LogicRPNNode::LogicRPNNode()
            : children{nullptr, nullptr}, op(nullptr), type(OpType::Invalid) {}

        LogicRPNNode::~LogicRPNNode() {
            delete children[0];
            delete children[1];
        }
        LogicRPNNode* Expressions::buildNestedLogicRPN(const ExpressionTokens& tokens) {
            std::stack<ExpressionToken*> opStack;       // Logic operators: &&, ||
            std::stack<LogicRPNNode*> outStack;         // Nested RPN nodes
            std::vector<ExpressionToken> calcBuffer;   // Arithmetic/comparison buffer

            auto flushCalcBuffer = [&]() {
                if (!calcBuffer.empty()) {
                    LogicRPNNode* leaf = new LogicRPNNode();
                    leaf->type = LogicRPNNode::OpType::CalcLeaf;
                    leaf->calcRPN = ToCalcRPN(calcBuffer); // Convert infix -> RPN
                    outStack.push(leaf);
                    calcBuffer.clear();
                }
            };

            auto applyOperator = [&]() {
                if (!opStack.empty() && outStack.size() >= 2) {
                    
                    ExpressionToken* op = opStack.top(); opStack.pop();
                    LogicRPNNode* right = outStack.top(); outStack.pop();
                    LogicRPNNode* left = outStack.top(); outStack.pop();
                    LogicRPNNode* parent = new LogicRPNNode();
                    parent->op = op;
                    parent->children[0] = left;
                    parent->children[1] = right;
                    //parent.children = {left, right};
                    switch (op->type) {
                        case TokenType::LogicalAnd: parent->type = LogicRPNNode::OpType::LogicalAnd; break;
                        case TokenType::LogicalOr:  parent->type = LogicRPNNode::OpType::LogicalOr;  break;
                        default:                    parent->type = LogicRPNNode::OpType::Invalid;    break;
                    }
                    outStack.push(parent);
                }
            };

            for (int i = 0; i < tokens.count; ++i) {
                ExpressionToken& tok = tokens.items[i];

                if (tok.type == TokenType::Ignore)
                    continue;

                if (tok.type == TokenType::LogicalAnd || tok.type == TokenType::LogicalOr) {
                    flushCalcBuffer();
                    while (!opStack.empty() && opStack.top()->type != TokenType::LeftParenthesis &&
                        LogicPrecedence(opStack.top()->type) >= LogicPrecedence(tok.type)) {
                        applyOperator();
                    }
                    opStack.push(&tok);//&tokens.items[i]);
                }
                else if (tok.type == TokenType::LeftParenthesis) {
                    opStack.push(&tok);//&tokens.items[i]);
                }
                else if (tok.type == TokenType::RightParenthesis) {
                    flushCalcBuffer();
                    while (!opStack.empty() && opStack.top()->type != TokenType::LeftParenthesis) {
                        applyOperator();
                    }
                    if (!opStack.empty() && opStack.top()->type == TokenType::LeftParenthesis) {
                        opStack.pop();
                    }
                }
                else {
                    // Arithmetic or comparison token
                    calcBuffer.push_back(tok);

                    // Flush when a comparison operator has both operands
                    if (calcBuffer.size() >= 3 &&
                        (tok.type == TokenType::Operand ||
                        tok.type == TokenType::CompareEqualsTo ||
                        tok.type == TokenType::CompareNotEqualsTo ||
                        tok.type == TokenType::CompareLessThan ||
                        tok.type == TokenType::CompareGreaterThan ||
                        tok.type == TokenType::CompareLessThanOrEqual ||
                        tok.type == TokenType::CompareGreaterThanOrEqual)) {
                        if (IsComparisonOperator(calcBuffer[calcBuffer.size() - 2].type)) {
                            flushCalcBuffer();
                        }
                    }
                }
            }

            flushCalcBuffer();
            while (!opStack.empty()) {
                if (opStack.top()->type != TokenType::LeftParenthesis)
                    applyOperator();
                else
                    opStack.pop();
            }

            return outStack.empty() ? nullptr : outStack.top();
        }


    }
}