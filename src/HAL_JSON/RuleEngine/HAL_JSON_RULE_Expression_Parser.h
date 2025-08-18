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

        struct LogicRPNNode {
            std::vector<ExpressionToken> calcRPN;  // leaf if op.empty()
            std::vector<LogicRPNNode> children;   // nested nodes
            ExpressionToken op;                     // "&&" or "||", empty for leaf
        };
        class Expressions {
        private:
            static void ReportError(const char* msg, const char* param = nullptr);
            static void ReportWarning(const char* msg, const char* param = nullptr);
            static void ReportInfo(std::string msg);

            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;
            
            static inline int CalcPrecedence(TokenType optype) {
                if (optype == TokenType::CalcMultiply || 
                    optype == TokenType::CalcDivide) return 6;
                if (optype == TokenType::CalcPlus || 
                    optype == TokenType::CalcMinus) return 5;
                if (optype == TokenType::CompareLessThan || 
                    optype == TokenType::CompareGreaterThan || 
                    optype == TokenType::CompareLessOrEqualsTo ||
                    optype == TokenType::CompareGreaterOrEqualsTo) return 4;
                if (optype == TokenType::CompareEqualsTo || 
                    optype == TokenType::CompareNotEqualsTo) return 3;
                return 0;
            }

            static inline bool IsCalcOperator(TokenType optype) {
                return CalcPrecedence(optype) > 0;
            }

            static inline int LogicPrecedence(TokenType optype) {
                if (optype == TokenType::LogicalOr) return 1;
                if (optype == TokenType::LogicalAnd) return 2;
                return 0;
            }

            static inline bool IsLogicOperator(TokenType optype) {
                return optype == TokenType::LogicalOr || optype == TokenType::LogicalAnd;
            }

            // Helper: returns true if c can be part of an identifier
            static inline bool IsIdentifierChar(char c) {
                return std::isalnum(c) || c == '_';
            }

            // Helper: returns true if c is a single-character operator
            static inline TokenType IsSingleOp(char c) {
                if (c == '+') return TokenType::CalcPlus;
                else if (c == '-') return TokenType::CalcMinus;
                else if (c == '*')  return TokenType::CalcMultiply;
                else if (c == '/') return TokenType::CalcDivide;
                else if (c == '<') return TokenType::CompareLessThan;
                else if (c == '>') return TokenType::CompareGreaterThan;
                return TokenType::NotSet;
            }

            // Helper: checks if c + next form a 2-char operator
            static inline TokenType IsTwoCharOp(char c, char next) {
                if (c == '&' && next == '&') return TokenType::LogicalAnd;
                else if (c == '|' && next == '|') return TokenType::LogicalOr;
                else if (c == '=' && next == '=') return TokenType::CompareEqualsTo;
                else if (c == '!' && next == '=') return TokenType::CompareNotEqualsTo;
                else if (c == '<' && next == '=') return TokenType::CompareLessOrEqualsTo;
                else if (c == '>' && next == '=') return TokenType::CompareGreaterOrEqualsTo;
                return TokenType::NotSet;
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

            static std::vector<ExpressionToken> ToCalcRPN(const std::vector<ExpressionToken>& tokens);

            static void printLogicRPNNode(const LogicRPNNode& node);
            static LogicRPNNode buildNestedLogicRPN(const ExpressionTokens& tokens);

            static int preParseTokensCount(const Tokens& rawTokens);
            static ExpressionTokens* preParseTokens(const Tokens& rawTokens);

        };
    }
}