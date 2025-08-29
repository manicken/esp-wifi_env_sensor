#pragma once

#include <Arduino.h>
#include "../HAL_JSON_ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Manager.h"
#include "HAL_JSON_RULE_Parser_Token.h"
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
            static inline TokenType IsSingleOp(char c) {
                if (c == '+') return TokenType::CalcPlus;
                else if (c == '-') return TokenType::CalcMinus;
                else if (c == '*')  return TokenType::CalcMultiply;
                else if (c == '/') return TokenType::CalcDivide;
                else if (c == '<') return TokenType::CompareLessThan;
                else if (c == '>') return TokenType::CompareGreaterThan;
                else if (c == '&') return TokenType::CalcBitwiseAnd;
                else if (c == '|') return TokenType::CalcBitwiseOr;
                else if (c == '^') return TokenType::CalcBitwiseExOr;
                else if (c == '&') return TokenType::CalcModulus;
                return TokenType::NotSet;
            }

            // Helper: checks if c + next form a 2-char operator
            static inline TokenType IsTwoCharOp(const char* c) {
                char first = *(c++);
                char next = *c;
                if (first == '&' && next == '&') return TokenType::LogicalAnd;
                else if (first == '|' && next == '|') return TokenType::LogicalOr;
                else if (first == '=' && next == '=') return TokenType::CompareEqualsTo;
                else if (first == '!' && next == '=') return TokenType::CompareNotEqualsTo;
                else if (first == '<' && next == '=') return TokenType::CompareLessThanOrEqual;
                else if (first == '>' && next == '=') return TokenType::CompareGreaterThanOrEqual;
                else if (first == '>' && next == '>') return TokenType::CalcBitwiseRightShift;
                else if (first == '<' && next == '<') return TokenType::CalcBitwiseLeftShift;
                return TokenType::NotSet;
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

            static void GetGenerateRPNTokensCount_PreCalc(const Tokens& rawTokens, int& totalCount, int& operatorCount);
            static int GetGenerateRPNTokensCount_DryRun(const Tokens& rawTokens, int initialSize);
           
            static LogicRPNNode* BuildLogicTree(ExpressionTokens* tokens);
            static ExpressionTokens* GenerateRPNTokens(const Tokens& rawTokens);
        };
    }
}