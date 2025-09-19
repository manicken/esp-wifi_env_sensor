/*

  <one line to give the program's name and a brief idea of what it does.>

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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