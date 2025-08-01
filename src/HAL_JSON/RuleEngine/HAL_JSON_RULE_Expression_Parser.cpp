
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

        bool Expressions::CountOperatorsAndOperands(const char* expr, int& operatorCount, int& operandCount, int& leftParenthesisCount ) {
            operatorCount = 0;
            operandCount = 0;
            leftParenthesisCount  = 0;
            // early checks
            if (expr == nullptr) {
                ReportError("expr. is nullptr");
                return false;
            }
            if (strlen(expr) == 0) {
                ReportError("expr. is empty");
                return false;
            }
            if(IsDoubleOperator(expr)) { // this only checks the two first characters in the Expression
                ReportError("expr. cannot start with a operator");
                return false;
            }
            if(IsSingleOperator(*expr)) {
                ReportError("expr. cannot start with a operator");
                return false;
            }

            int rightParenthesisCount = 0;

            bool inOperand = false;
            
            for (const char* p = expr; *p != '\0'; p++) {
                if (IsDoubleOperator(p)) {
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
        
        void Expressions::GetOperands(const char* str, ZeroCopyString* operands, int operandCount) {

            bool inOperand = false;
            int operandIndex = 0;
            const char* p = str;
            for ( ; *p != '\0' ; ++p) {
                if (IsDoubleOperator(p)) {
                    
                    if (inOperand) {
                        if (operandIndex == operandCount) { // out of bounds should never happend
                            ReportError("something is very wrong");
                        } else
                            operands[operandIndex++].end = p;
                        
                    }
                    p++;
                    inOperand = false;
                }
                else if (IsSingleOperator(*p) || *p == '(' || *p == ')') {
                    if (inOperand) {
                        if (operandIndex == operandCount) { // out of bounds should never happend
                            ReportError("something is very wrong");
                        } else
                            operands[operandIndex++].end = p;
                        
                    }
                    inOperand = false;
                }
                else if (!inOperand) {
                    operands[operandIndex].start = p;
                    inOperand = true;
                }
            }
            // catch the last operand if any
            if (inOperand) {
                if (operandIndex == operandCount) { // out of bounds should never happend
                    ReportError("something is very wrong");
                } else {
                    operands[operandIndex].end = p;
                }
            }
        }

        bool Expressions::OperandIsVariable(const ZeroCopyString& operand) {
            const char* p = operand.start;
            const char* const end = operand.end;
            while (p < end) {
                if (isdigit(*p) == false) return true;
                p++;
            }
            return false;
        }

        const char* Expressions::ValidOperandVariableName(const ZeroCopyString& operand) {
            const char* p = operand.start;
            const char* const end = operand.end;
            while (p < end) {
                if (IsValidOperandChar(*p) == false) {
                    return p;
                }
                p++;
            }
            return nullptr;
        }
        
        bool Expressions::ValidateExpression(const char* str) {
            int operatorCount, operandCount, leftParenthesisCount ;
            bool anyError = false;

            anyError = (false == CountOperatorsAndOperands(str, operatorCount, operandCount, leftParenthesisCount));
            // allways print debug info
            ReportInfo("operatorCount:" + std::to_string(operatorCount) + ", operandCount:" + std::to_string(operandCount) + ", leftParenthesisCount :" + std::to_string(leftParenthesisCount));
            // early return as if this happens then there is no point of continuing
            if (anyError) return false; 

            ZeroCopyString* operands = new ZeroCopyString[operandCount];

            GetOperands(str, operands, operandCount);
#ifdef HAL_JSON_RULES_EXPRESSIONS_PARSER_SHOW_DEBUG
            // just print debug info
            for (int i=0;i<operandCount;i++) {
                ZeroCopyString operand = operands[i]; // create 'copy'
                std::string msg;
                if (OperandIsVariable(operand)) {
                    msg = "Variable - Name: ";
                    ZeroCopyString funcName = operand;
                    operand = funcName.SplitOffHead('#');
                    msg += operand.ToString();
                    if (funcName.Length() != 0) {
                        msg += ", funcName: ";
                        msg += funcName.ToString();
                    }
                } else {
                    msg = "Const Value: ";
                    msg += operand.ToString();
                }
                ReportInfo(msg);
            }
#endif

            // first check if variable names are valid as it could be a good idea
            // to not allow every character for better looking code
            bool foundAnyInvalidChar = false;
            for (int i=0;i<operandCount;i++) {
                const ZeroCopyString& operand = operands[i];

                if (OperandIsVariable(operand)) {
                    const char* currChar = ValidOperandVariableName(operand);
                    if (currChar != nullptr) {
                        char invalidChar[2];
                        invalidChar[0] = *currChar;
                        invalidChar[1] = 0x00;
                        // don't know yet if it should be warning or error
                        //ReportError("found invalid character in operand",invalidChar);
                        ReportWarning("found invalid character in operand",invalidChar);
                        foundAnyInvalidChar = true;
                    } 
                }
            }
            if (foundAnyInvalidChar) {
                // here we can return early or check the rest for additional errors
            }
            for (int i=0;i<operandCount;i++) {
                const ZeroCopyString& operand = operands[i];

                if (OperandIsVariable(operand)) {
                    ZeroCopyString varOperand = operand;
                    // here we need to check if the variablename is a (logical/physical/virtual) device
                    ZeroCopyString funcName = varOperand;
                    varOperand = funcName.SplitOffHead('#');
                    if (varOperand.Length() > HAL_UID::Size) {
                        std::string msg = varOperand.ToString() + " length > " + std::to_string(HAL_UID::Size);
                        ReportError("Operand name too long",msg.c_str());
                        anyError = true;
                    }
                    UIDPath path(varOperand);

                    // 1. check if the device exists
                    Device* device = Manager::findDevice(path);
                    if (device == nullptr) {
                        std::string deviceName = varOperand.ToString();
                        ReportError("could not find the device:", deviceName.c_str());
                        anyError = true;
                        continue;
                    }
                    // 2a. if funcname we verify both that the device supports read and that the funcname is valid
                    if (funcName.Length() != 0) {
                        HALValue halValue;
                        HALReadValueByCmd readValueByCmd(halValue, funcName);
                        if (device->read(readValueByCmd) == false) {
                            std::string funcNameStr = funcName.ToString();
                            ReportError("could not read the device by cmd:", funcNameStr.c_str());
                            anyError = true;
                            continue;
                        }
                    }
                    // 2b. here we only check if the device can be read
                    else {
                        HALValue halValue;
                        if (device->read(halValue) == false) {
                            ReportError("this device do not support read");
                            anyError = true;
                            continue;
                        }
                    }
                    
                }
            }
            delete[] operands;
            return anyError == false;
        }
    }
}