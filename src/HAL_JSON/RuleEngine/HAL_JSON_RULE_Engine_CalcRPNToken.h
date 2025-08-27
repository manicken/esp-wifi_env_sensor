
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_Value.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_CachedDeviceAccess.h"
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_RPNStack.h"
#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {
        /** used for all value calculations */ 
        extern RPNStack<HALValue> halValueStack;

        struct CalcRPNToken {
            HAL_JSON_NOCOPY_NOMOVE(CalcRPNToken);
            /** 
             * in this case this will either be:
             * CachedDeviceAccess
             * HALValue
             */
            void* context;
            HALOperationResult (*handler)(void* context);
            Deleter deleter;

            static HALOperationResult GetAndPushVariableValue_Handler(void* context);
            static HALOperationResult GetAndPushConstValue_Handler(void* context);

            static HALOperationResult Add_Operation_Handler(void* context);
            static HALOperationResult Subtract_Operation_Handler(void* context);
            static HALOperationResult Multiply_Operation_Handler(void* context);
            static HALOperationResult Divide_Operation_Handler(void* context);
            static HALOperationResult Modulus_Operation_Handler(void* context);
            static HALOperationResult BitwiseAnd_Operation_Handler(void* context);
            static HALOperationResult BitwiseOr_Operation_Handler(void* context);
            static HALOperationResult BitwiseExOr_Operation_Handler(void* context);
            static HALOperationResult BitwiseLeftShift_Operation_Handler(void* context);
            static HALOperationResult BitwiseRightShift_Operation_Handler(void* context);

            static HALOperationResult NotEquals_Operation_Handler(void* context);
            static HALOperationResult Equals_Operation_Handler(void* context);
            static HALOperationResult LessThan_Operation_Handler(void* context);
            static HALOperationResult LargerThan_Operation_Handler(void* context);
            static HALOperationResult LessThanOrEquals_Operation_Handler(void* context);
            static HALOperationResult LargerThanOrEquals_Operation_Handler(void* context);

            CalcRPNToken();
            ~CalcRPNToken();
        };
    }
}