#pragma once

#include <Arduino.h>
#include "../../../src/Support/ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../../../src/HAL_JSON/HAL_JSON_UID_Path.h"

#ifdef _WIN32
#include <iostream>
#endif

namespace HAL_JSON {
    namespace Rules {
        class Expressions {
        private:
            static const char* operatorList;
            static void ReportError(const char* msg);
            static void ReportInfo(std::string msg);

            static bool IsOperator(char c);
            static bool CountOperatorsAndOperands(const char* expr, int& operatorCount, int& operandCount, int& leftParenthesisCount );
            static void GetOperands(const char* str, ZeroCopyString* operands, int operandCount);
            static bool OperandIsVariable(const ZeroCopyString& operand);
        public:
            static bool ValidateExpression(const char* str);
        };
    }
}