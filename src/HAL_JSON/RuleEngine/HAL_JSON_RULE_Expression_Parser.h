#pragma once

#include <Arduino.h>
#include "../HAL_JSON_ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Manager.h"
#include "HAL_JSON_RULE_Parser_Token.h"
#include "HAL_JSON_RULE_Expression_Token.h"
#include "HAL_JSON_RULE_Engine_Support.h"

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

        struct LogicRPNNode {

            std::vector<ExpressionToken*> calcRPN;  // leaf if op.empty()
            /** is owned and needs to be deleted */
            LogicRPNNode* childA;   // nested nodes, nullptr when leaf
            /** is owned and needs to be deleted */
            LogicRPNNode* childB;   // nested nodes, nullptr when leaf
            /** this is non owned, it's owned by the input token stream */
            ExpressionToken* op;                     // "&&" or "||", nullptr when leaf

            LogicRPNNode();
            ~LogicRPNNode();
        };
        
        class Expressions {
        private:
            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;
        public:
            static void ReportError(const char* msg, const char* param = nullptr);
            static void ReportWarning(const char* msg, const char* param = nullptr);
            static void ReportInfo(std::string msg);

            // Helper: returns true if c is a single-character operator
            static inline ExpTokenType IsSingleOp(char c) {
                if (c == '+') return ExpTokenType::CalcPlus;
                else if (c == '-') return ExpTokenType::CalcMinus;
                else if (c == '*')  return ExpTokenType::CalcMultiply;
                else if (c == '/') return ExpTokenType::CalcDivide;
                else if (c == '<') return ExpTokenType::CompareLessThan;
                else if (c == '>') return ExpTokenType::CompareGreaterThan;
                else if (c == '&') return ExpTokenType::CalcBitwiseAnd;
                else if (c == '|') return ExpTokenType::CalcBitwiseOr;
                else if (c == '^') return ExpTokenType::CalcBitwiseExOr;
                else if (c == '&') return ExpTokenType::CalcModulus;
                return ExpTokenType::NotSet;
            }

            // Helper: checks if c + next form a 2-char operator
            static inline ExpTokenType IsTwoCharOp(const char* c) {
                char first = *(c++);
                char next = *c;
                if (first == '&' && next == '&') return ExpTokenType::LogicalAnd;
                else if (first == '|' && next == '|') return ExpTokenType::LogicalOr;
                else if (first == '=' && next == '=') return ExpTokenType::CompareEqualsTo;
                else if (first == '!' && next == '=') return ExpTokenType::CompareNotEqualsTo;
                else if (first == '<' && next == '=') return ExpTokenType::CompareLessThanOrEqual;
                else if (first == '>' && next == '=') return ExpTokenType::CompareGreaterThanOrEqual;
                else if (first == '>' && next == '>') return ExpTokenType::CalcBitwiseRightShift;
                else if (first == '<' && next == '<') return ExpTokenType::CalcBitwiseLeftShift;
                return ExpTokenType::NotSet;
            }
            
            //static void GetOperands(Tokens& tokens, ZeroCopyString* operands, int operandCount);
            
            /** returns nullptr if no invalid char is found, otherwise it will return the character */
            
        public:
            static const char* ValidOperandVariableName(const Token& operand);
            static bool CountOperatorsAndOperands(Tokens& tokens, int& operatorCount, int& operandCount, int& leftParenthesisCount, ExpressionContext exprContext);
            static void ValidateOperand(const Token& operand, bool& anyError, ValidateOperandMode mode = ValidateOperandMode::Read);
            //static bool OperandIsVariable(const Token& operand);
            static bool IsSingleOperator(char c);
            static bool IsDoubleOperator(const char* c);
            static bool IsValidOperandChar(char c);
            static bool ValidateExpression(Tokens& tokens, ExpressionContext exprContext);

            static void printLogicRPNNodeTree(LogicRPNNode* node, int indent = 0);
            static void PrintLogicRPNNode(const LogicRPNNode* node, int depth = 0);

            static void GetGenerateRPNTokensCount_PreCalc(const Tokens& rawTokens, int& totalCount, int& operatorCount);
            static int GetGenerateRPNTokensCount_DryRun(const Tokens& rawTokens, int initialSize);
           
            static LogicRPNNode* BuildLogicTree(ExpressionTokens* tokens);
            static ExpressionTokens* GenerateRPNTokens(Tokens& rawTokens);
        };
    }
}