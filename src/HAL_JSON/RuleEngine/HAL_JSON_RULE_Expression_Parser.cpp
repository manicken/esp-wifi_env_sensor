
#include "HAL_JSON_RULE_Expression_Parser.h"
#include <unordered_map>
#include <cctype>  // for isspace, isdigit, isalpha

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
            std::cout << msg;
#else
            //GlobalLogger.Info(F("Expr Rule Parse:"), msg);
#endif
        }

        // precedence map
        static const std::unordered_map<ExpTokenType, int> precedence = {
            // calc
            {ExpTokenType::CalcMultiply, 7}, {ExpTokenType::CalcDivide, 7}, {ExpTokenType::CalcModulus, 7},
            {ExpTokenType::CalcPlus, 6}, {ExpTokenType::CalcMinus, 6},
            {ExpTokenType::CalcBitwiseLeftShift, 5}, {ExpTokenType::CalcBitwiseRightShift, 5},
            {ExpTokenType::CalcBitwiseAnd, 4}, {ExpTokenType::CalcBitwiseExOr, 4}, {ExpTokenType::CalcBitwiseOr, 4},

            // compare
            {ExpTokenType::CompareGreaterThan, 3}, {ExpTokenType::CompareLessThan, 3},
            {ExpTokenType::CompareGreaterThanOrEqual, 3}, {ExpTokenType::CompareLessThanOrEqual, 3},
            {ExpTokenType::CompareEqualsTo, 2}, {ExpTokenType::CompareNotEqualsTo, 2},

            // logic
            {ExpTokenType::LogicalAnd, 1}, {ExpTokenType::LogicalOr, 0}
        };

        inline int getPrecedence(ExpTokenType t) {
            auto it = precedence.find(t);
            return (it != precedence.end()) ? it->second : -1;
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

        std::string CalcExpressionToString(const std::vector<ExpressionToken*>& calcExpr) {
            std::string out;
            for (int i=0;i<calcExpr.size();i++) {
                out += calcExpr[i]->ToString();
                if (i + 1 < calcExpr.size()) out += " ";
            }
            return out;
        }

        void Expressions::printLogicRPNNodeTree(LogicRPNNode* node, int indent) {
            if (!node) return;

            std::string padding(indent * 2, ' '); // 2 spaces per level

            if (node->op == nullptr || node->op->IsEmpty()) {
                // Leaf node → print calc RPN
                ReportInfo(padding + "- calc: [" + CalcExpressionToString(node->calcRPN) + "]\n");

            } else {
                // Operator node

                ReportInfo(padding + "[" + (node->op?ExpTokenTypeToString(node->op->type):"") + "]\n");

                if (node->childA) printLogicRPNNodeTree(node->childA, indent + 1);
                if (node->childB) printLogicRPNNodeTree(node->childB, indent + 1);
             }
        }
        
        void PrintExpressionTokens(std::vector<ExpressionToken*>& calcRPN) {
            Expressions::ReportInfo("\nTokens: ");
            for (int i=0;i<calcRPN.size();i++){
                Expressions::ReportInfo(calcRPN[i]->ToString() + " ");
            }
            Expressions::ReportInfo("\n");
        }

        void Expressions::GetGenerateRPNTokensCount_PreCalc(const Tokens& rawTokens, int& totalCount, int& operatorCount) {
            int totalCountTemp = 0;
            int operatorCountTemp = 0;
            
            for (int i = 0; i < rawTokens.count; ++i) {
                const Token& token = rawTokens.items[i];
                int j = 0;
                const int tokenStrLength = token.Length();
                while (j < tokenStrLength) {
                    char c = token[j];
                    if (c == '(') {
                        operatorCountTemp++;
                        ++j;
                    } else if (c == ')') {                      
                        ++j;
                    } else if ((j + 1) < tokenStrLength && IsTwoCharOp(token.start + j) != ExpTokenType::NotSet) {
                        ++totalCountTemp; j += 2;
                        operatorCountTemp++;
                    } else if (IsSingleOp(c) != ExpTokenType::NotSet) {
                        ++totalCountTemp; ++j;
                        operatorCountTemp++;
                    } else {
                        // identifier/number
                        int startIdx = j;
                        while (j < tokenStrLength &&
                            token[j] != '(' &&
                            token[j] != ')' &&
                            IsSingleOp(token[j]) == ExpTokenType::NotSet &&
                            !((j + 1) < tokenStrLength && IsTwoCharOp(token.start + j) != ExpTokenType::NotSet)) {
                            ++j;
                        }
                        ++totalCountTemp;
                    }
                }
            }
            totalCount = totalCountTemp;
            operatorCount = operatorCountTemp;
        }
        int Expressions::GetGenerateRPNTokensCount_DryRun(const Tokens& rawTokens, int initialSize)
        {
            int maxOperatorUsage = 0;  // track maximum usage for this expression

            // Dry-run op stack (store only TokenType, not full ExpressionToken*)
            ExpTokenType* opStack = new ExpTokenType[initialSize];
            int opStackIndex = 0;      // current operator stack depth

            const int rawTokensCount = rawTokens.count;
            for (int i = 0; i < rawTokensCount; ++i) {
                const Token& token = rawTokens.items[i];
                const int tokenStrLength = token.Length();

                for (int j = 0; j < tokenStrLength; ++j) {
                    char c = token[j];

                    // Parentheses
                    if (c == '(') {
                        opStack[opStackIndex++] = ExpTokenType::LeftParenthesis;

                        if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                    }
                    else if (c == ')') {
                        // Pop until LeftParenthesis is found
                        while (opStackIndex != 0) {
                            ExpTokenType top = opStack[opStackIndex - 1];
                            if (top == ExpTokenType::LeftParenthesis) break;
                            opStackIndex--;
                        }
                        if (opStackIndex != 0)
                            opStackIndex--; // discard LeftParenthesis
                    }
                    else {
                        // Detect operator type
                        ExpTokenType twoCharOpType = ExpTokenType::NotSet;
                        if ((j + 1) < tokenStrLength) twoCharOpType = IsTwoCharOp(token.start + j);

                        if (twoCharOpType != ExpTokenType::NotSet) {
                            // Pop higher or equal precedence operators
                            while (opStackIndex != 0) {
                                ExpTokenType top = opStack[opStackIndex - 1];
                                if (top == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top) < getPrecedence(twoCharOpType)) break;
                                opStackIndex--;
                            }
                            opStack[opStackIndex++] = twoCharOpType;
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                            j++; // consume extra char
                            continue;
                        }

                        // Single-character operator
                        ExpTokenType oneCharOpType = IsSingleOp(c);
                        if (oneCharOpType != ExpTokenType::NotSet) {
                            while (opStackIndex != 0) {
                                ExpTokenType top = opStack[opStackIndex - 1];
                                if (top == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top) < getPrecedence(oneCharOpType)) break;
                                opStackIndex--;
                            }
                            opStack[opStackIndex++] = oneCharOpType;
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                        }
                        else {
                            // Operand / identifier
                            while (j < tokenStrLength &&
                                token[j] != '(' &&
                                token[j] != ')' &&
                                IsSingleOp(token[j]) == ExpTokenType::NotSet &&
                                !((j + 1) < tokenStrLength && IsTwoCharOp(token.start + j) != ExpTokenType::NotSet)) {
                                ++j;
                            }
                            j--; // adjust for outer loop increment
                        }
                    }
                }
            }
            delete[] opStack;
            return maxOperatorUsage;
        }

        ExpressionTokens* Expressions::GenerateRPNTokens(const Tokens& rawTokens) {
            
            int totalCount = 0;
            int operatorCount = 0;
            int maxOperatorUsage = 0;
            GetGenerateRPNTokensCount_PreCalc(rawTokens, totalCount, operatorCount);
            // Second pass: simulate the operator stack precisely
            // This step is intended to run during the validation phase,
            // where globally 'static' reusable stacks for both RPN output and operator stack
            // should be created in the Expressions namespace.
            // For now, during development, we keep the stacks local here.
            operatorCount = GetGenerateRPNTokensCount_DryRun(rawTokens, operatorCount);
            
            ExpressionTokens* outTokens = new ExpressionTokens(totalCount);
            ExpressionToken** outTokenItems = outTokens->items;

            ExpressionToken** opStack = new ExpressionToken*[operatorCount];
            int opStackIndex = 0;

            int outTokensIndex = 0;
            const int rawTokensCount = rawTokens.count;
            for (int i = 0; i < rawTokensCount; ++i) {
                const Token& token = rawTokens.items[i];

                const int tokenStrLength = token.Length();
                for (int j=0; j < tokenStrLength; j++) {
                    char c = token[j];

                    // Parentheses
                    if (c == '(') {
                        opStack[opStackIndex++] = new ExpressionToken(token.start+j, 1, ExpTokenType::LeftParenthesis);
                        if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                    }
                    else if ( c == ')') {
                        // Pop until LeftParenthesis is found
                        while (opStackIndex != 0)
                        {
                            ExpressionToken* top = opStack[opStackIndex - 1];
                            if (top->type == ExpTokenType::LeftParenthesis) break;
                            outTokenItems[outTokensIndex++] = top;
                            opStackIndex--;
                        }

                        if (opStackIndex != 0)
                            opStackIndex--; // discard the LeftParenthesis
                        else
                            ReportError("Mismatched parenthesis"); // should never happend
                    }
                    else {
                        ExpTokenType twoCharOpType = ExpTokenType::NotSet;
                        if ((j + 1) < tokenStrLength) twoCharOpType = IsTwoCharOp(token.start + j);
                        
                        if (twoCharOpType != ExpTokenType::NotSet) {
                            
                            // While there's an operator on top of the opStack with greater or equal precedence
                            while (opStackIndex != 0)
                            {
                                ExpressionToken* top = opStack[opStackIndex - 1];
                                if (top->type == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top->type) < getPrecedence(twoCharOpType)) break;
                                outTokenItems[outTokensIndex++] = top; 
                                opStackIndex--;
                            }
                            // Push current operator as new item
                            opStack[opStackIndex++] = new ExpressionToken(token.start+j, 2, twoCharOpType);
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                            j++; // consume one extra token
                            continue;
                        }

                        // Single-character operator
                        ExpTokenType oneCharOpType = IsSingleOp(c);
                        if (oneCharOpType != ExpTokenType::NotSet) {
                            
                            // While there's an operator on top of the opStack with greater or equal precedence
                            while (opStackIndex != 0)
                            {
                                ExpressionToken* top = opStack[opStackIndex - 1];
                                if (top->type == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top->type) < getPrecedence(oneCharOpType)) break;
                                outTokenItems[outTokensIndex++] = top;
                                opStackIndex--;
                            }
                            // Push current operator as new item
                            opStack[opStackIndex++] = new ExpressionToken(token.start + j, 1, oneCharOpType);
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                        }
                        else {
                            // Identifier / number
                            int startIdx = j;
                            for (;j < tokenStrLength;j++) {
                                if (token[j] == '(' || token[j] == ')') break;
                                if (((j + 1) < tokenStrLength && IsTwoCharOp(token.start + j)!=ExpTokenType::NotSet)) break;
                                if (IsSingleOp(token[j]) != ExpTokenType::NotSet) break;
                            }
                            outTokenItems[outTokensIndex++] = new ExpressionToken(token.start + startIdx, token.start + j, ExpTokenType::Operand);
                            j--; 

                        }
                    }
/*
                    ReportInfo("outStack:");
                    for (int i=0;i<outTokensIndex;i++) {
                        ReportInfo(outTokenItems[i]->ToString() + " ");
                    }
                    ReportInfo("\n");
                    ReportInfo("opStack:");
                    for (int i=0;i<opStackIndex;i++) {
                        ReportInfo(opStack[i]->ToString() + " ");
                    }
                    ReportInfo("\n");
*/
                }
            }
            // After loop: pop remaining ops
            while (opStackIndex != 0)
                outTokenItems[outTokensIndex++] = opStack[--opStackIndex];
            ReportInfo("\nGenerateRPNTokens used: " + std::to_string(outTokensIndex) + " of " + std::to_string(totalCount) + "\n");
            ReportInfo("\nGenerateRPNTokens used op: " + std::to_string(maxOperatorUsage) + " of " + std::to_string(operatorCount) + "\n");

            delete[] opStack;
            outTokens->index = outTokensIndex;
            return outTokens;
        }

        LogicRPNNode::LogicRPNNode()
            : childA(nullptr), childB(nullptr), op(nullptr)/*, type(OpType::Invalid)*/ {}

        LogicRPNNode::~LogicRPNNode() {
            delete childA;
            delete childB;
        }
        
        static const ExpTokenType compareOperators[] = {ExpTokenType::CompareEqualsTo, ExpTokenType::CompareNotEqualsTo,
                                                     ExpTokenType::CompareGreaterThanOrEqual, ExpTokenType::CompareLessThanOrEqual,
                                                     ExpTokenType::CompareGreaterThan, ExpTokenType::CompareLessThan, 
                                                     ExpTokenType::NotSet};

        LogicRPNNode* Expressions::BuildLogicTree(ExpressionTokens* tokens)
        {
            std::vector<LogicRPNNode*> stack;

            auto makeCalc = [&](std::vector<ExpressionToken*> rpn) {
                auto* n = new LogicRPNNode();
                n->calcRPN = std::move(rpn);
                n->childA = nullptr;
                n->childB = nullptr;
                n->op = nullptr;
                return n;
            };

            auto makeOp = [&](ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs) {
                auto* n = new LogicRPNNode();
                n->childA = lhs;
                n->childB = rhs;
                n->op = opTok;
                return n;
            };

            std::vector<ExpressionToken*> currentCalc;
            int tokensCount = tokens->count;
            for (int i=0;i<tokensCount;i++) {
                ExpressionToken& tok = *tokens->items[i];
                if (tok.type == ExpTokenType::LogicalAnd || tok.type == ExpTokenType::LogicalOr) {
                    // first flush pending calc as a leaf
                    if (!currentCalc.empty()) {
                        stack.push_back(makeCalc(currentCalc));
                        currentCalc.clear();
                    }

                    if (stack.size() < 2) // this should never happend
                        throw std::runtime_error("LogicRPN: not enough operands for logic op");

                    auto rhs = stack.back(); stack.pop_back();
                    auto lhs = stack.back(); stack.pop_back();
                    stack.push_back(makeOp(&tok, lhs, rhs));
                }
                else {
                    currentCalc.push_back(&tok);

                    // detect end of a comparison (= leaf boundary)
                    if (tok.AnyType(compareOperators)) {
                        stack.push_back(makeCalc(currentCalc));
                        currentCalc.clear();
                    }
                }
            }

            if (!currentCalc.empty()) {
                stack.push_back(makeCalc(currentCalc));
                currentCalc.clear();
            }

            if (stack.size() != 1)
                throw std::runtime_error("LogicRPN: unbalanced tree");

            return stack.back();
        }
    }
}