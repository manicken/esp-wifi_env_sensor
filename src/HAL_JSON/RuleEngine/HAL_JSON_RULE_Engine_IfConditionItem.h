
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RUL_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"

namespace HAL_JSON {
    namespace Rule {

        struct IfConditionItem {
            /** this is executed on the calc results of lhsItems and rhsItems */
            bool (*handler)(const HALValue& lhs, const HALValue& rhs);

            CalcRPNToken* lhsItems;
            int lhsCount;
            CalcRPNToken* rhsItems;
            int rhsCount;

            IfConditionItem(IfConditionItem&) = delete;          // no copy constructor
            IfConditionItem& operator=(const IfConditionItem&) = delete; // no copy assignment
            IfConditionItem(IfConditionItem&& other) = delete;           // no move constructor
            IfConditionItem& operator=(IfConditionItem&& other) = delete; // no move assignment

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