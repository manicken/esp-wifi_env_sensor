
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

        ExpressionTokens* Expressions::rpnOutputStack = nullptr;
        int Expressions::rpnOutputStackNeededSize = 0;
        ExpressionToken* Expressions::opStack = nullptr;
        int Expressions::opStackSizeNeededSize = 0;
        int Expressions::opStackSize = 0;

        void Expressions::CalcStackSizesInit() {
            rpnOutputStackNeededSize = 0;
            opStackSizeNeededSize = 0;

        }
        void Expressions::CalcStackSizes(Tokens& tokens) {
            int totalCount = 0;
            int operatorCount = 0;
            GetGenerateRPNTokensCount_PreCalc(tokens, totalCount, operatorCount);
            // Second pass: simulate the operator stack precisely
            operatorCount = GetGenerateRPNTokensCount_DryRun(tokens, operatorCount);

            if (opStackSize > opStackSizeNeededSize) opStackSizeNeededSize = opStackSize;
            if (totalCount > rpnOutputStackNeededSize) rpnOutputStackNeededSize = totalCount;
        }
        void Expressions::InitStacks() {
            delete rpnOutputStack;
            delete opStack;
            rpnOutputStack = new ExpressionTokens(rpnOutputStackNeededSize);
            opStack = new ExpressionToken[opStackSize];
        }
        void Expressions::ClearStacks() {
            delete rpnOutputStack;
            delete opStack;
        }
        // precedence map
        static const std::unordered_map<ExpTokenType, int> precedence = {
            // calc
            {ExpTokenType::CalcMultiply, 8}, {ExpTokenType::CalcDivide, 8}, {ExpTokenType::CalcModulus, 8},
            {ExpTokenType::CalcPlus, 7}, {ExpTokenType::CalcMinus, 7},
            {ExpTokenType::CalcBitwiseLeftShift, 6}, {ExpTokenType::CalcBitwiseRightShift, 6},
            {ExpTokenType::CalcBitwiseAnd, 5}, {ExpTokenType::CalcBitwiseExOr, 5}, {ExpTokenType::CalcBitwiseOr, 5},

            // compare
            {ExpTokenType::CompareGreaterThan, 4}, {ExpTokenType::CompareLessThan, 4},
            {ExpTokenType::CompareGreaterThanOrEqual, 4}, {ExpTokenType::CompareLessThanOrEqual, 4},
            {ExpTokenType::CompareEqualsTo, 3}, {ExpTokenType::CompareNotEqualsTo, 3},

            // logic
            {ExpTokenType::LogicalAnd, 2}, {ExpTokenType::LogicalOr, 1}

            // assigment

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
                const char* tokenEnd = token.end;
                for (const char* p = effectiveStart; p < tokenEnd; p++) {
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
                const char* tokenEnd = token.end;
                for (p = effectiveStart ; p < tokenEnd; ++p) {
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

        std::string CalcExpressionToString(const ExpressionTokens* calcExpr) {
            std::string out;
            if (calcExpr == nullptr){
                Expressions::ReportError("calcExpr == nullptr");
                return out;
            }
            int length = calcExpr->count;
            ExpressionToken* calcExprItems = calcExpr->items;
            for (uint32_t i=0;i<length;i++) {
                out += calcExprItems[i].ToString();
                if (i + 1 < length) out += " ";
            }
            return out;
        }

        void Expressions::printLogicRPNNodeTree(LogicRPNNode* node, int indent) {
            if (!node) return;
            std::string padding(indent * 2, ' '); // 2 spaces per level
            if (node->calcRPN != nullptr) {
                // Leaf node → print calc RPN
                ReportInfo(padding + "- calc: [" + CalcExpressionToString(node->calcRPN) + "]\n");
            } else {
                // Operator node
                ReportInfo(padding + "[" + (node->op?ExpTokenTypeToString(node->op->type):"") + "]\n");
                if (node->childA) printLogicRPNNodeTree(node->childA, indent + 1);
                if (node->childB) printLogicRPNNodeTree(node->childB, indent + 1);
             }
        }

        void Expressions::PrintLogicRPNNodeAdvancedTree(const LogicRPNNode* node, int depth) {
            if (!node) {
                ReportInfo(std::string(depth * 2, ' ') + "Node: nullptr\n");
                return;
            }
            std::string indent(depth * 2, ' ');
            // Print operator
            if (node->op) {
                ReportInfo(indent + "op: " + node->op->ToString() + "\n");  
            } else {
                ReportInfo(indent + "op: nullptr\n");
            }
            // Print calcRPN tokens
            ReportInfo(indent + "calcRPN: ");
            if (node->calcRPN == nullptr) {
                ReportInfo("empty\n");
            } else {
                int length = node->calcRPN->count;
                ExpressionToken* calcRPNitems = node->calcRPN->items;
                for (int i=0;i<length;i++) {
                    ReportInfo(calcRPNitems[i].ToString() + " ");
                }
                ReportInfo("\n");
            }
            // Print children
            ReportInfo(indent + "childA:\n");
            if (node->childA)
                PrintLogicRPNNodeAdvancedTree(node->childA, depth + 1);
            else
                ReportInfo(indent + "  nullptr\n");

            ReportInfo(indent + "childB:\n");
            if (node->childB)
                PrintLogicRPNNodeAdvancedTree(node->childB, depth + 1);
            else
                ReportInfo(indent + "  nullptr\n");
        }

        void Expressions::GetGenerateRPNTokensCount_PreCalc(const Tokens& tokens, int& totalCount, int& operatorCount) {
            int totalCountTemp = 0;
            int operatorCountTemp = 0;

            const int startIndex = tokens.currIndex;
            const int endIndex = startIndex + tokens.Current().itemsInBlock;
            for (int cti = startIndex; cti < endIndex; cti++) {
                const Token& token = tokens.items[cti];
                const char* tokenStart = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    tokenStart  = tokens.firstTokenStartOffset;
                    //ReportInfo("***************** firstTokenStartOffset was set\n"); 
                } else {
                    tokenStart  = token.start;
                }
                const char* tokenEnd = token.end;

                for (const char* cPtr=tokenStart; cPtr < tokenEnd; cPtr++) {
                    
                    if (*cPtr == '(') {
                        operatorCountTemp++;
                    } else if (*cPtr == ')') {                      
                        
                    } else if ((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr) != ExpTokenType::NotSet) {
                        ++totalCountTemp;
                        operatorCountTemp++;
                        cPtr++; // consume the extra char
                    } else if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) {
                        ++totalCountTemp;
                        operatorCountTemp++;
                    } else {
                        // identifier/number
                        while (cPtr < tokenEnd) {
                            if (*cPtr == '(') break;
                            if (*cPtr == ')') break;
                            if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) break;;
                            if ((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr) != ExpTokenType::NotSet) break;
                            cPtr++;
                        }
                        cPtr--; // adjust for outer loop
                        ++totalCountTemp;
                    }
                }
            }
            totalCount = totalCountTemp;
            operatorCount = operatorCountTemp;
        }
        int Expressions::GetGenerateRPNTokensCount_DryRun(const Tokens& tokens, int initialSize)
        {
            int maxOperatorUsage = 0;  // track maximum usage for this expression

            // Dry-run op stack (store only TokenType, not full ExpressionToken*)
            ExpTokenType* opStack = new ExpTokenType[initialSize];
            int opStackIndex = 0;      // current operator stack depth
            const int startIndex = tokens.currIndex;
            const int endIndex = startIndex + tokens.Current().itemsInBlock;
            for (int cti = startIndex; cti < endIndex; cti++) {
                const Token& token = tokens.items[cti];
                const char* tokenStart = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    tokenStart  = tokens.firstTokenStartOffset;
                    //ReportInfo("***************** firstTokenStartOffset was set\n"); 
                } else {
                    tokenStart  = token.start;
                }
                const char* tokenEnd = token.end;

                for (const char* cPtr=tokenStart; cPtr < tokenEnd; cPtr++) {
                    //char c = token[j];

                    // Parentheses
                    if (*cPtr == '(') {
                        opStack[opStackIndex++] = ExpTokenType::LeftParenthesis;

                        if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                    }
                    else if (*cPtr == ')') {
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
                        if ((cPtr + 1) < tokenEnd) twoCharOpType = IsTwoCharOp(cPtr);

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
                            cPtr++; // consume extra char
                            continue;
                        }

                        // Single-character operator
                        ExpTokenType oneCharOpType = IsSingleOp(*cPtr);
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
                            while (cPtr < tokenEnd) {
                                if (*cPtr == '(') break;
                                if (*cPtr == ')') break;
                                if ((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr) != ExpTokenType::NotSet) break;
                                if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) break;
                                cPtr++;
                            }
                            cPtr--; // adjust for outer loop increment
                        }
                    }
                }
            }
            delete[] opStack;
            return maxOperatorUsage;
        }

        ExpressionTokens* Expressions::GenerateRPNTokens(Tokens& tokens) {
            
            int totalCount = 0;
            int operatorCount = 0;
            /** development test only */
            int maxOperatorUsage = 0;
            GetGenerateRPNTokensCount_PreCalc(tokens, totalCount, operatorCount);
            // Second pass: simulate the operator stack precisely
            // This step is intended to run during the validation phase,
            // where globally 'static' reusable stacks for both RPN output and operator stack
            // should be created in the Expressions namespace.
            // For now, during development, we keep the stacks local here.
            operatorCount = GetGenerateRPNTokensCount_DryRun(tokens, operatorCount);
            
            ExpressionTokens* outTokens = new ExpressionTokens(totalCount);
            ExpressionToken* outTokenItems = outTokens->items;

            ExpressionToken* opStack = new ExpressionToken[operatorCount];
            int opStackIndex = 0;

            int outTokensIndex = 0;
            
            const int endIndex = tokens.currIndex + tokens.Current().itemsInBlock;
            const int startindex = tokens.currIndex;
            // consume current tokens here so we don't forget and to clearly mark what is happend
            tokens.currIndex = endIndex; 
            for (int cti = startindex; cti < endIndex; cti++) {
                const Token& token = tokens.items[cti];
                
                const char* tokenStart = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    tokenStart  = tokens.firstTokenStartOffset;
                    //ReportInfo("***************** firstTokenStartOffset was set\n"); 
                } else {
                    tokenStart  = token.start;
                }
                const char* tokenEnd = token.end;
                //const int tokenStrLength = token.Length();
                for (const char* cPtr=tokenStart; cPtr < tokenEnd; cPtr++) {
                    
                    // Parentheses
                    if (*cPtr == '(') {
                        // Push current operator as 'new' item
                        opStack[opStackIndex++].Set(cPtr, 1, ExpTokenType::LeftParenthesis);
                        if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                    }
                    else if (*cPtr == ')') {
                        // Pop until LeftParenthesis is found
                        while (opStackIndex != 0)
                        {
                            ExpressionToken& top = opStack[opStackIndex - 1];
                            if (top.type == ExpTokenType::LeftParenthesis) break;
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
                        if ((cPtr + 1) < tokenEnd) twoCharOpType = IsTwoCharOp(cPtr);
                        
                        if (twoCharOpType != ExpTokenType::NotSet) {
                            
                            // While there's an operator on top of the opStack with greater or equal precedence
                            while (opStackIndex != 0)
                            {
                                ExpressionToken& top = opStack[opStackIndex - 1];
                                if (top.type == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top.type) < getPrecedence(twoCharOpType)) break;
                                outTokenItems[outTokensIndex++] = top;
                                opStackIndex--;
                            }
                            // Push current operator as 'new' item
                            opStack[opStackIndex++].Set(cPtr, 2, twoCharOpType);
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                            cPtr++; // consume one extra token
                            continue;
                        }

                        // Single-character operator
                        ExpTokenType oneCharOpType = IsSingleOp(*cPtr);
                        if (oneCharOpType != ExpTokenType::NotSet) {
                            
                            // While there's an operator on top of the opStack with greater or equal precedence
                            while (opStackIndex != 0)
                            {
                                ExpressionToken& top = opStack[opStackIndex - 1];
                                if (top.type == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top.type) < getPrecedence(oneCharOpType)) break;
                                outTokenItems[outTokensIndex++] = top;
                                opStackIndex--;
                            }
                            // Push current operator as 'new' item
                            opStack[opStackIndex++].Set(cPtr, 1, oneCharOpType);
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                        }
                        else {
                            // Identifier / number
                            //int startIdx = j;
                            const char* start = cPtr;
                            for (;cPtr < tokenEnd;cPtr++) {
                                if (*cPtr == '(' || *cPtr == ')') break;
                                if (((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr)!=ExpTokenType::NotSet)) break;
                                if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) break;
                            }
                            outTokenItems[outTokensIndex++].Set(start, cPtr, ExpTokenType::Operand);
                            cPtr--; // as the current loop will incr after this 

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
            ReportInfo("\nGenerateRPNTokens used: " + std::to_string(outTokensIndex) + " of " + std::to_string(outTokens->count) + "\n");
            ReportInfo("\nGenerateRPNTokens used op: " + std::to_string(maxOperatorUsage) + " of " + std::to_string(operatorCount) + "\n");

            outTokens->currentCount = outTokensIndex;

            // define if this expression contains logic operators
            // this allows the tree builder to have a single ptr to the rpnArray as the context
            outTokens->containLogicOperators = false;
            // going backwards as that is the fastest way of detecting logic operators
            // as they mostly appear at the end
            for (int i=outTokensIndex-1;i>=0;i--) { 
                ExpressionToken& tok = outTokens->items[i];
                if (tok.type == ExpTokenType::LogicalAnd || tok.type == ExpTokenType::LogicalOr) {
                    outTokens->containLogicOperators = true;
                    break; //  we only need to check if there is one
                }
            }

            delete[] opStack; // this should not be done when we utilize global stack
            return outTokens;
        }

        
        
        static const ExpTokenType compareOperators[] = {ExpTokenType::CompareEqualsTo, ExpTokenType::CompareNotEqualsTo,
                                                     ExpTokenType::CompareGreaterThanOrEqual, ExpTokenType::CompareLessThanOrEqual,
                                                     ExpTokenType::CompareGreaterThan, ExpTokenType::CompareLessThan, 
                                                     ExpTokenType::NotSet};
        
        /** 
         * Development test functions
         * TODO. make a copy of them that produce the exec format
         */
        LogicRPNNode* Expressions::BuildLogicTree(ExpressionTokens* tokens)
        {
            int tokensCount = tokens->count;

            // in the final to exec function
            // the following should be calculated 
            // in the scripts validate stage and stored globally 
            // so that it can allocate the memory globally before all exec structure is made
            // thus reducing heap fragmentation
            int stackMaxSize = 0;
            for (int i=0;i<tokensCount;i++) {
                ExpressionToken& tok = tokens->items[i];
                // include logic operators here for safetly
                if (tok.AnyType(compareOperators) || tok.type == ExpTokenType::LogicalAnd || tok.type == ExpTokenType::LogicalOr) stackMaxSize++;
            }
            LogicRPNNode** stack = new LogicRPNNode*[stackMaxSize]();
            int stackIndex = 0;
  
            int currentCalcStartIndex = -1;
            
            for (int i=0;i<tokensCount;i++) {

                ExpressionToken& tok = tokens->items[i];
                if (tok.type == ExpTokenType::LogicalAnd || tok.type == ExpTokenType::LogicalOr) {
                    // first flush pending calc as a leaf
                    if (currentCalcStartIndex != -1) {
                        LogicRPNNode* newNode = new LogicRPNNode(tokens, currentCalcStartIndex, i); // here it should only be i as the logic operator should not be included
                        stack[stackIndex++] = newNode; // push item
                        currentCalcStartIndex = -1; // clear
                    }
                    ReportInfo("BuildLogicTree stack size:" + std::to_string(stackIndex) + "\n");
                    // This should never happen under normal use.
                    // It can occur if logical and arithmetic expressions are improperly combined,
                    // e.g., (a == 0 || b == 1) + 2, which is invalid.
                    if (stackIndex < 2)
                        throw std::runtime_error("LogicRPN: not enough operands for logic op");

                    auto rhs = stack[--stackIndex];// stack.back(); stack.pop_back();
                    auto lhs = stack[--stackIndex];//stack.back(); stack.pop_back();
                    LogicRPNNode* newNode =  new LogicRPNNode(&tok, lhs, rhs);
                    stack[stackIndex++] = newNode; // push item
                }
                else {
                    if (currentCalcStartIndex == -1) currentCalcStartIndex = i;
                    

                    // detect end of a comparison (= leaf boundary)
                    if (tok.AnyType(compareOperators)) {
                        LogicRPNNode* newNode = new LogicRPNNode(tokens, currentCalcStartIndex, i+1); // i+1 as the compare operator needs to be included
                        stack[stackIndex++] = newNode; // push item
                        currentCalcStartIndex = -1; // clear
                    }
                }
            }

            if (currentCalcStartIndex != -1) {
                LogicRPNNode* newNode = new LogicRPNNode(tokens, currentCalcStartIndex, tokensCount);
                stack[stackIndex++] = newNode; // push item
                currentCalcStartIndex = -1; // clear
            }

            if (stackIndex != 1)
                throw std::runtime_error("LogicRPN: unbalanced tree");
            LogicRPNNode* ret = stack[stackIndex-1];
            delete[] stack;
            return ret;
        }
    }
}