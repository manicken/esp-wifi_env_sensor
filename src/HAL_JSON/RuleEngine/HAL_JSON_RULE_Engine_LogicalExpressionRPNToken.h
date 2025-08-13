
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_RPNStack.h"
#include "HAL_JSON_RULE_Engine_IfConditionItem.h"

namespace HAL_JSON {
    namespace Rule {

        /** used for logical conditions */
        extern RPNStack<bool> boolValueStack;

        struct LogicalExpressionRPNToken {
            HAL_JSON_NOCOPY_NOMOVE(LogicalExpressionRPNToken);

            HALOperationResult (*handler)(IfConditionItem* context);

            IfConditionItem* context;

            LogicalExpressionRPNToken();
            ~LogicalExpressionRPNToken();
        };
    }
}