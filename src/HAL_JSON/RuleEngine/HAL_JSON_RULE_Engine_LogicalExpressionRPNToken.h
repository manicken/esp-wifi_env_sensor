
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RUL_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_RPNStack.h"
#include "HAL_JSON_RULE_Engine_IfConditionItem.h"

namespace HAL_JSON {
    namespace Rule {

        /** used for logical conditions */
        extern RPNStack<bool> boolValueStack;

        struct LogicalExpressionRPNToken {
            HALOperationResult (*handler)(LogicalExpressionRPNToken* context);

            IfConditionItem* context;

            LogicalExpressionRPNToken(LogicalExpressionRPNToken&) = delete;          // no copy constructor
            LogicalExpressionRPNToken& operator=(const LogicalExpressionRPNToken&) = delete; // no copy assignment
            LogicalExpressionRPNToken(LogicalExpressionRPNToken&& other) = delete;           // no move constructor
            LogicalExpressionRPNToken& operator=(LogicalExpressionRPNToken&& other) = delete; // no move assignment

            LogicalExpressionRPNToken();
            ~LogicalExpressionRPNToken();
        };
    }
}