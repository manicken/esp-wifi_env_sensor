#pragma once

#include <Arduino.h>
#include "../../../src/Support/ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Manager.h"

#ifdef _WIN32
#include <iostream>
#endif

#define HAL_JSON_RULES_EXPRESSIONS_SINGLE_OPERATOR_LIST "+-*/|&^><"
#define HAL_JSON_RULES_EXPRESSIONS_DOUBLE_OPERATOR_LIST "&&" "||" "==" "!=" ">=" "<="

namespace HAL_JSON {
    namespace Rules {
        class Expressions {
        private:
            static void ReportError(const char* msg, const char* param = nullptr);
            static void ReportWarning(const char* msg, const char* param = nullptr);
            static void ReportInfo(std::string msg);

            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;
            static bool IsSingleOperator(char c);
            static bool IsDoubleOperator(const char* c);
            static bool IsValidOperandChar(char c);

            static bool CountOperatorsAndOperands(const char* expr, int& operatorCount, int& operandCount, int& leftParenthesisCount );
            static void GetOperands(const char* str, ZeroCopyString* operands, int operandCount);
            static bool OperandIsVariable(const ZeroCopyString& operand);
            /** returns nullptr if no invalid char is found, otherwise it will return the character */
            static const char* ValidOperandVariableName(const ZeroCopyString& operand);
        public:
            static bool ValidateExpression(const char* str);
        };
    }
}