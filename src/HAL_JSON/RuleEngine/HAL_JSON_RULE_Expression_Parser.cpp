
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
            if (c == nullptr || *(c + 1) == '\0') return false; // safety

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
            if(IsDoubleOperator(tokens.items[0].start) && exprContext == ExpressionContext::IfCondition) { // this only checks the two first characters in the Expression
                ReportError("expr. cannot start with a operator");
                return false;
            }
            if(IsSingleOperator(*tokens.items[0].start)) {
                ReportError("expr. cannot start with a operator");
                return false;
            }

            int rightParenthesisCount = 0;

            bool inOperand = false;

            for (int cti=0;cti<tokens.count;cti++) { // cti = currTokenIndex
                for (const char* p = tokens.items[cti].start; p < tokens.items[cti].end; p++) {
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
                    std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }
                for (p = effectiveStart ; p < token.end; ++p) {
                    if (IsDoubleOperator(p)&& exprContext == ExpressionContext::IfCondition) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-token.start);
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
                            operand.column = token.column + (operandStart-token.start);
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
                    operand.column = token.column + (operandStart-token.start);
                    ValidateOperand(operand, anyError);
                    ++operandIndex;
                    inOperand = false;
                }
            }
            return !anyError;
        }

        void Expressions::ValidateOperand(const Token& operandToken, bool& anyError) {
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
            HALDeviceOperationResult readResult = HALDeviceOperationResult::UnsupportedOperation;
            if (funcName.Length() != 0) {
                HALValue halValue;
                HALReadValueByCmd readValueByCmd(halValue, funcName);
                readResult = device->read(readValueByCmd);
                if (readResult != HALDeviceOperationResult::Success) {
                    anyError = true;
                    if (readResult == HALDeviceOperationResult::UnsupportedCommand) {
                        std::string funcNameStr = ": " + funcName.ToString();
                        ReportTokenError(operandToken, ToString(readResult), funcNameStr.c_str());
                    } else {
                        ReportTokenError(operandToken, ToString(readResult), ": read");
                    }
                    
                }
            } else {
                HALValue halValue;
                readResult = device->read(halValue);
                if (readResult != HALDeviceOperationResult::Success) {
                    ReportTokenError(operandToken, ToString(readResult), ": read");
                    anyError = true;
                }
            }
        }
    }
}