#pragma once

#include <Arduino.h>
#include "../HAL_JSON_Value.h"
namespace HAL_JSON {

    struct OpAdd { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a + b; } };
    struct OpSub { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a - b; } };
    struct OpMul { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a * b; } };
    struct OpDiv { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a / b; } };
    struct OpMod { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a % b; } };
    // bitwise
    struct OpBitOr { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a | b; } };
    struct OpBitAnd { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a & b; } };
    struct OpBitExOr { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a ^ b; } };
    struct OpBitLshift { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a << b; } };
    struct OpBitRshift { inline static HALValue apply(const HALValue& a, const HALValue& b) { return a >> b; } };
    // compare operators
    struct OpCompEqual { inline static HALValue apply(const HALValue& a, const HALValue& b) { return (a == b); } };
    struct OpCompNotEqual { inline static HALValue apply(const HALValue& a, const HALValue& b) { return (a != b); } };
    struct OpCompLessOrEqual { inline static HALValue apply(const HALValue& a, const HALValue& b) { return (a <= b); } };
    struct OpCompGreaterOrEqual { inline static HALValue apply(const HALValue& a, const HALValue& b) { return (a >= b); } };
    struct OpCompLessThan { inline static HALValue apply(const HALValue& a, const HALValue& b) { return (a < b); } };
    struct OpCompGreaterThan { inline static HALValue apply(const HALValue& a, const HALValue& b) { return (a > b); } };
}