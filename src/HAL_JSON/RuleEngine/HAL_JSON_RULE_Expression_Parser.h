#pragma once

#include <Arduino.h>
#include "../HAL_JSON_ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Manager.h"
#include "HAL_JSON_RULE_Parser_Token.h"

#include <string>

#ifdef _WIN32
#include <iostream>
#endif

#define HAL_JSON_RULES_EXPRESSIONS_PARSER_SHOW_DEBUG

#define HAL_JSON_RULES_EXPRESSIONS_SINGLE_OPERATOR_LIST "+-*/|&^><"
#define HAL_JSON_RULES_EXPRESSIONS_DOUBLE_OPERATOR_LIST "&&" "||" "==" "!=" ">=" "<="

#define HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD "\\"

#include <vector> // until we see it working

namespace HAL_JSON {
    namespace Rules {
        enum class ExpressionContext {
            IfCondition,
            Assignment
        };
        enum class ValidateOperandMode {
            Read,
            Write,
            ReadWrite
        };
        struct CalcRPN {
            std::vector<ZeroCopyString> tokens;   // postfix order
        };

        struct LogicRPN {
            std::vector<CalcRPN> operands;        // each operand is one sub-expression
            std::vector<ZeroCopyString> ops;      // &&, || in postfix order
        };

        struct LogicRPNNode {
            std::vector<ZeroCopyString> calcRPN;  // leaf if op.empty()
            std::vector<LogicRPNNode> children;   // nested nodes
            ZeroCopyString op;                     // "&&" or "||", empty for leaf
        };
        class Expressions {
        private:
            static void ReportError(const char* msg, const char* param = nullptr);
            static void ReportWarning(const char* msg, const char* param = nullptr);
            static void ReportInfo(std::string msg);

            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;
            
            static inline int CalcPrecedence(const ZeroCopyString& op) {
                if (op == "*" || op == "/") return 6;
                if (op == "+" || op == "-") return 5;
                if (op == "<" || op == "<=" || op == ">" || op == ">=") return 4;
                if (op == "==" || op == "!=") return 3;
                return 0;
            }

            static inline bool IsCalcOperator(const ZeroCopyString& op) {
                return CalcPrecedence(op) > 0;
            }

            static inline int LogicPrecedence(const ZeroCopyString& op) {
                if (op == "||") return 1;
                if (op == "&&") return 2;
                return 0;
            }

            static inline bool IsLogicOperator(const ZeroCopyString& op) {
                return op == "&&" || op == "||";
            }
            
            //static void GetOperands(Tokens& tokens, ZeroCopyString* operands, int operandCount);
            
            /** returns nullptr if no invalid char is found, otherwise it will return the character */
            
        public:
            static const char* ValidOperandVariableName(const Token& operand);
            static bool CountOperatorsAndOperands(Tokens& tokens, int& operatorCount, int& operandCount, int& leftParenthesisCount, ExpressionContext exprContext);
            static void ValidateOperand(const Token& operand, bool& anyError, ValidateOperandMode mode = ValidateOperandMode::Read);
            static bool OperandIsVariable(const Token& operand);
            static bool IsSingleOperator(char c);
            static bool IsDoubleOperator(const char* c);
            static bool IsValidOperandChar(char c);
            static bool ValidateExpression(Tokens& tokens, ExpressionContext exprContext);

            static LogicRPN BuildRPN(const Tokens& tokens);
            static CalcRPN ToCalcRPN(const std::vector<ZeroCopyString>& tokens);

            static void printLogicRPNNode(const LogicRPNNode& node);
            static LogicRPNNode buildNestedLogicRPN(const Tokens& tokens);

        };
    }
}