#pragma once

#include <Arduino.h>
#include "../HAL_JSON_ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Manager.h"
#include "HAL_JSON_RULE_Parser_Token.h"
#include "HAL_JSON_RULE_Engine_Support.h"

#include <string>

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
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

            bool IsPureCalcNode();
        };

        void SliceStackReportError(const char* msg, const char* param);

        template<typename T, const char* TypeName>
        struct SliceStack {
            static constexpr const char* Name = TypeName;
            T* data;
            int minIndex;     // beginning of current slice
            int currIndex;    // next free slot
            int stackSize;
            int maxUsage;

            inline SliceStack() = delete;

            inline SliceStack(int _size) {
                data = new T[_size];
                stackSize = _size;
                minIndex = 0;
                currIndex = 0;
                maxUsage = 0;
            }
            inline ~SliceStack() {
                delete[] data;
            }
            // push
            inline void push(const T& v) {
                if (currIndex >= stackSize) {
                    SliceStackReportError("push - stack overflow:",Name);
                    return;
                }
                data[currIndex++] = v;
                if (currIndex > maxUsage)
                    maxUsage = currIndex;
            }

            // pop
            inline T top_n_pop() {
                if (currIndex == minIndex) {
                    SliceStackReportError("top_n_pop - stack underflow:",Name);
                    return nullptr;
                }
                return data[--currIndex];
            }

            inline void pop() {
                if (currIndex == minIndex) {
                    SliceStackReportError("pop - stack underflow:",Name);
                    return;
                }
                currIndex--;
            }
            inline void collapseTop() {
                if (currIndex - minIndex < 2) {
                    SliceStackReportError("pop - stack underflow:",Name);
                    return;
                }
                data[currIndex-2] = data[currIndex-1];
                currIndex--;
            }

            // top
            inline T& top() {
                if (currIndex == minIndex) {
                    SliceStackReportError("top - stack underflow:",Name);
                    static T dummy{}; // reference to dummy
                    return dummy;
                }
                return data[currIndex - 1];
            }

            // size of *this slice*
            inline int size() const {
                return currIndex - minIndex;
            }

            // check empty
            inline bool empty() const {
                return currIndex == minIndex;
            }

            // check not empty
            inline bool notEmpty() const {
                return currIndex != minIndex;
            }

            inline void ClearCurrSlice() {
                currIndex = minIndex;
            }

            // restore slice (like leaving it)
            inline void Restore(int prevMin, int prevCurr) {
                minIndex = prevMin;
                currIndex = prevCurr;
            }

            // start a nested slice (local context)
            inline void BeginSlice(int& _currMinIndex, int& _currIndex) {
                _currIndex = currIndex;
                _currMinIndex = minIndex;
                minIndex = currIndex;
            }

            inline T& operator[](int idx) const {
                if (idx < 0 || idx >= currIndex - minIndex) {
                    SliceStackReportError("stack out of bounds",Name);
                    static T dummy{}; // reference to dummy
                    return dummy;
                }
                return data[minIndex + idx];
            }
        };
        constexpr char PrintName[] = "print";
        constexpr char OpName[] = "op";
        constexpr char OutName[] = "out";
        constexpr char TempName[] = "temp";
        struct ParseContext {
            HAL_JSON_NOCOPY_NOMOVE(ParseContext);
         
            SliceStack<ExpressionToken*, OpName> opStack;
            SliceStack<ExpressionToken*, TempName> tempStack;
            SliceStack<LogicRPNNode*, OutName> outStack;

            ParseContext() = delete; // prevents misuse
            ParseContext(int opStackSize, int outStackSize, int tempStackSize);
            
            void merge_calc_from(LogicRPNNode* node);
            void FlushTempToNode(LogicRPNNode* node);

            void FlushCalc();
            void ApplyOperator();

            std::string PrintTempStackSlice();
            
        };
        class Expressions {
        private:
            

            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;
        public:
            static void ReportError(const char* msg, const char* param = nullptr);
            static void ReportWarning(const char* msg, const char* param = nullptr);
            static void ReportInfo(std::string msg);
            static inline int CalcPrecedence(TokenType optype) {
                if (optype == TokenType::CalcMultiply || 
                    optype == TokenType::CalcDivide) return 6;
                if (optype == TokenType::CalcPlus || 
                    optype == TokenType::CalcMinus) return 5;
                if (optype == TokenType::CompareLessThan || 
                    optype == TokenType::CompareGreaterThan || 
                    optype == TokenType::CompareLessThanOrEqual ||
                    optype == TokenType::CompareGreaterThanOrEqual) return 4;
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
                else if (c == '&') return TokenType::CalcBitwiseAnd;
                else if (c == '|') return TokenType::CalcBitwiseOr;
                else if (c == '^') return TokenType::CalcBitwiseExOr;
                else if (c == '&') return TokenType::CalcModulus;
                return TokenType::NotSet;
            }

            // Helper: checks if c + next form a 2-char operator
            static inline TokenType IsTwoCharOp(char c, char next) {
                if (c == '&' && next == '&') return TokenType::LogicalAnd;
                else if (c == '|' && next == '|') return TokenType::LogicalOr;
                else if (c == '=' && next == '=') return TokenType::CompareEqualsTo;
                else if (c == '!' && next == '=') return TokenType::CompareNotEqualsTo;
                else if (c == '<' && next == '=') return TokenType::CompareLessThanOrEqual;
                else if (c == '>' && next == '=') return TokenType::CompareGreaterThanOrEqual;
                else if (c == '>' && next == '>') return TokenType::CalcBitwiseRightShift;
                else if (c == '<' && next == '<') return TokenType::CalcBitwiseLeftShift;
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

            static std::vector<ExpressionToken> ToCalcRPN(const std::vector<ExpressionToken>& tokens);
            static std::vector<ExpressionToken> ToCalcRPN(const std::vector<ExpressionToken*>& tokens);
            static void InplaceCalcRPN(std::vector<ExpressionToken*>& tokens);

            static void printLogicRPNNode(const LogicRPNNode* node);
            static void printLogicRPNNodeTree(LogicRPNNode* node, int indent = 0);

            static void preParseTokensCount(const Tokens& rawTokens, int& totalCount, int* operatorCount=nullptr);
            static int RPNcountOpTokensDryRun(const Tokens& rawTokens, int initialSize);
            static ExpressionTokens* preParseTokens(const Tokens& rawTokens);
 
            /** to use this function preParseTokens is needed to be run before */
            static LogicRPNNode* ParseConditionalExpression(ExpressionTokens& tokens, ParseContext& ctx);
            static LogicRPNNode* ParseConditionalExpression2(ExpressionTokens& tokens, ParseContext& ctx);
            
            /** this should be run at root after ParseConditionalExpression */
            static void DoAllInplaceCalcRPN(LogicRPNNode* node);


            static void ParseConditionalExpression(ExpressionTokens& tokens,std::vector<ExpressionToken*>& tempStack);
            static LogicRPNNode* BuildLogicTree(const std::vector<ExpressionToken*>& tokens);
            static ExpressionTokens* GenerateRPNTokens(const Tokens& rawTokens);
        };
    }
}