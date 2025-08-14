
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"

namespace HAL_JSON {
    namespace Rules {

        struct IfConditionItem {
            HAL_JSON_NOCOPY_NOMOVE(IfConditionItem);

            /** this is executed on the calc results of lhsItems and rhsItems */
            bool (*handler)(const HALValue& lhs, const HALValue& rhs);

            CalcRPNToken* lhsItems;
            int lhsCount;
            CalcRPNToken* rhsItems;
            int rhsCount;

            static bool NotEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
            static bool Equals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
            static bool LessThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
            static bool LargerThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
            static bool LessThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
            static bool LargerThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);

            IfConditionItem();
            ~IfConditionItem();
        };
    }
}