
#include "HAL_JSON_RULE_Expression_Parser.h"

namespace HAL_JSON {
    namespace Rules {
        void Expressions::ReportError(const char* msg) {
#ifdef _WIN32
            std::cout << "Error: " << msg << std::endl;
#else
            GlobalLogger.Error(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportInfo(std::string msg) {
#ifdef _WIN32
            std::cout << msg << std::endl;
#else
            //GlobalLogger.Info(F("Expr Rule Parse:"), msg);
#endif
        }

        const char* Expressions::operatorList = "+-*/|&^";

        bool Expressions::IsOperator(char c) {
            const char* op = operatorList;
            while (*op != '\0') {
                if (*op == c) return true;
                ++op;
            }
            return false;
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
            if(IsOperator(*expr)) {
                ReportError("expr. cannot start with a operator");
                return false;
            }

            int rightParenthesisCount = 0;

            bool inOperand = false;
            
            for (const char* p = expr; *p != '\0'; ++p) {
                if (IsOperator(*p)) {
                    operatorCount++;
                    inOperand = false;
                } else if (*p == '(') {
                    leftParenthesisCount++;
                    inOperand = false;
                } else if (*p == ')') {
                    rightParenthesisCount++;
                    inOperand = false;
                }
                else if (!inOperand) {
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
                if (IsOperator(*p) || *p == '(' || *p == ')') {
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
        
        bool Expressions::ValidateExpression(const char* str) {
            int operatorCount, operandCount, leftParenthesisCount ;
            bool anyError = false;

            anyError = (false == CountOperatorsAndOperands(str, operatorCount, operandCount, leftParenthesisCount));
            // allways print debug info
            ReportInfo("\noperatorCount:" + std::to_string(operatorCount) + ", operandCount:" + std::to_string(operandCount) + ", leftParenthesisCount :" + std::to_string(leftParenthesisCount));
            if (anyError) return false;
            ZeroCopyString* operands = new ZeroCopyString[operandCount];

            GetOperands(str, operands, operandCount);
            // just debug info
            for (int i=0;i<operandCount;i++) {
                const ZeroCopyString& operand = operands[i];
                ReportInfo(std::string("isVar:") + (OperandIsVariable(operand)?"true":"false") + ", operand: " + operand.ToString());
            }
            ReportInfo("\n");
            for (int i=0;i<operandCount;i++) {
                const ZeroCopyString& operand = operands[i];

                if (OperandIsVariable(operand)) {
                    ZeroCopyString varOperand = operand;
                    // here we need to check if the variablename is a (logical/physical/virtual) device
                    const char* funcNameSeparator = varOperand.FindChar('#');
                    ZeroCopyString funcName;
                    if (funcNameSeparator) {
                        funcName.start = funcNameSeparator+1;
                        funcName.end = varOperand.end;
                        varOperand.end = funcNameSeparator;
                    }
                    ReportInfo("varOperand before: " + varOperand.ToString() + ", ");
                    UIDPath path(varOperand);
                    ReportInfo("uidPath decoded: " + path.ToString());
                    if (funcNameSeparator) {
                        ReportInfo(", funcName: " + funcName.ToString());
                    }
                    ReportInfo("\n");
                    // now we can actually 
                    // 1. check if the device exists
                    // 2. also if a device exists in a expression that 
                    //    also mean that that device MUST support read function
                    //    so we also need to check if the device read function
                    //    returns true
                    // 3. if funcName is provided we can also verify that
                    //
                    // if either above fail then the rule that this expresssion normally 
                    // belongs to should be rejected as invalid 
                }
            }


            delete[] operands;
            return true;
        }
    }
}