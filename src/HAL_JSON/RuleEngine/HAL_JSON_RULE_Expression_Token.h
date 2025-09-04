#pragma once

#include <Arduino.h> // Needed for String class
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash
#include <chrono>
#endif
#include <string>
#include <cstdint>

#include <vector>
#include <stack>

#include "../../Support/Logger.h"
#include "../../Support/ConvertHelper.h"
#include "../../Support/CharArrayHelpers.h"
#include "../HAL_JSON_ZeroCopyString.h"

namespace HAL_JSON {
    namespace Rules {
        
        enum class ExpTokenType : uint16_t {
            /** used to make it easier to see unset tokens, is also used as terminator item when defining a list of token types */
            NotSet,
            ConstValOperand,
            VarOperand,
            LeftParenthesis,
            RightParenthesis,
            LogicalAnd,
            LogicalOr,
            CompareEqualsTo,
            CompareNotEqualsTo,
            CompareLessThan,
            CompareGreaterThan,
            CompareLessThanOrEqual,
            CompareGreaterThanOrEqual,
            CalcPlus,
            CalcMinus,
            CalcMultiply,
            CalcDivide,
            CalcModulus,
            CalcBitwiseAnd,
            CalcBitwiseOr,
            CalcBitwiseExOr,
            CalcBitwiseLeftShift,
            CalcBitwiseRightShift
        };
        const char* ExpTokenTypeToString(ExpTokenType type) ;

        struct ExpressionToken : public ZeroCopyString {
            using ZeroCopyString::ZeroCopyString;
            ExpTokenType type;
            //int matchingIndex;
            bool AnyType(const ExpTokenType* candidates);
            ExpressionToken();
            ExpressionToken(const char* start, int length, ExpTokenType type);
            ExpressionToken(const char* _start, const char* _end, ExpTokenType _type);
            void Set(const char* start, int length, ExpTokenType type);
            void Set(const char* _start, const char* _end, ExpTokenType _type);
            ~ExpressionToken();
        };
        bool EqualsAny(ExpTokenType type, const ExpTokenType* candidates);
        struct ExpressionTokens {
            ExpressionToken* items;
            int currentCount;
            int count;
            bool containLogicOperators;
            ExpressionTokens();
            ExpressionTokens(int count);
            ~ExpressionTokens();
        };
        std::string PrintExpressionTokens(ExpressionTokens& _tokens, int start = 0, int end = -1); // -1 mean defaults to _tokens.count
    }
}