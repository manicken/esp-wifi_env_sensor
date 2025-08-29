
#include "HAL_JSON_RULE_Expression_Token.h"

namespace HAL_JSON {
    namespace Rules {
        const char* ExpTokenTypeToString(ExpTokenType type) {
            switch (type) {
                case ExpTokenType::NotSet: return "NotSet";
                case ExpTokenType::LeftParenthesis: return "LeftParenthesis";
                case ExpTokenType::RightParenthesis: return "RightParenthesis";
                case ExpTokenType::LogicalAnd: return "LogicalAnd";
                case ExpTokenType::LogicalOr: return "LogicalOr";
                case ExpTokenType::CompareEqualsTo: return "CompareEqualsTo";
                case ExpTokenType::CompareNotEqualsTo: return "CompareNotEqualsTo";
                case ExpTokenType::CompareLessThan: return "CompareLessThan";
                case ExpTokenType::CompareGreaterThan: return "CompareGreaterThan";
                case ExpTokenType::CompareLessThanOrEqual: return "CompareLessThanOrEqual";
                case ExpTokenType::CompareGreaterThanOrEqual: return "CompareGreaterThanOrEqual";
                case ExpTokenType::CalcPlus: return "CalcPlus";
                case ExpTokenType::CalcMinus: return "CalcMinus";
                case ExpTokenType::CalcMultiply: return "CalcMultiply";
                case ExpTokenType::CalcDivide: return "CalcDivide";
                case ExpTokenType::CalcModulus: return "CalcModulus";
                case ExpTokenType::CalcBitwiseAnd: return "CalcBitwiseAnd";
                case ExpTokenType::CalcBitwiseOr: return "CalcBitwiseOr";
                case ExpTokenType::CalcBitwiseExOr: return "CalcBitwiseExOr";
                case ExpTokenType::CalcBitwiseLeftShift: return "CalcBitwiseLeftShift";
                case ExpTokenType::CalcBitwiseRightShift: return "CalcBitwiseRightShift";
                case ExpTokenType::Operand: return "Operand";
                default: return "Unknown";
            }
        }

        ExpressionToken::ExpressionToken() : type(ExpTokenType::NotSet) {

        }
        ExpressionToken::ExpressionToken(const char* _start, const char* _end, ExpTokenType _type) {
            start = _start;
            end = _end;
            type = _type;
        }
        ExpressionToken::ExpressionToken(const char* _start, int length, ExpTokenType _type) {
            start = _start;
            end = _start + length;
            type = _type;
        }
        ExpressionToken::~ExpressionToken() {
            // nothing to free here
        }
        bool ExpressionToken::AnyType(const ExpTokenType* candidates) {
            while(*candidates != ExpTokenType::NotSet) {
                if (type == *candidates) return true;
                candidates++;
            }
            return false;
        }
        void ExpressionToken::Set(const char* _start, int length, ExpTokenType _type) {
            start = _start;
            end = _start + length;
            type = _type;
        }
        void ExpressionToken::Set(const char* _start, const char* _end, ExpTokenType _type) {
            start = _start;
            end = _end;
            type = _type;
        }
        void ExpressionToken::Set(ExpressionToken& other) {
            start = other.start;
            end = other.end;
            type = other.type;
        }

        ExpressionTokens::ExpressionTokens() {
            items = nullptr;
            count = 0;
            index = 0;
        }
        ExpressionTokens::ExpressionTokens(int _count) {
            count = _count;
            items = new ExpressionToken[_count];
            
            index = 0;
        }
        ExpressionTokens::~ExpressionTokens() {
            delete[] items;
            items = nullptr;
        }
        
        std::string PrintExpressionTokens(ExpressionTokens& _tokens, int start, int end) {
            ExpressionToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (end == -1) end = tokenCount;
 
            for (int i=start;i<end;i++) {
                ExpressionToken& tok = tokens[i];
                std::string msgLine = "Token[" + std::to_string(i) + "]  ";
                if (tok.type != ExpTokenType::Operand) {
                    msgLine += "---";
                    msgLine += ExpTokenTypeToString(tok.type);
                    msgLine += "---";
                } else {
                    msgLine += tok.ToString();
                }
                msg += msgLine + "\n";
            }
            return msg;
        }
    }
}