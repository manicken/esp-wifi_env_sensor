
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_Value.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_CachedDeviceAccess.h"
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_RPNStack.h"
#include "HAL_JSON_RULE_Parser_Token.h"
#include "HAL_JSON_RULE_Expression_Token.h"

namespace HAL_JSON {
    namespace Rules {
        /** used for all value calculations */ 
        extern RPNStack<HALValue> halValueStack;
        using RPNHandler = HALOperationResult(*)(void*);

        struct CalcRPNToken {
            HAL_JSON_NOCOPY_NOMOVE(CalcRPNToken);
            /** 
             * in this case this will either be:
             * CachedDeviceAccess
             * HALValue
             */
            void* context;
            RPNHandler handler;
            Deleter deleter;

            static HALOperationResult GetAndPushVariableValue_Handler(void* context);
            static HALOperationResult GetAndPushConstValue_Handler(void* context);

            static HALOperationResult Calc_Addition_Operation_Handler(void* context);
            static HALOperationResult Calc_Subtract_Operation_Handler(void* context);
            static HALOperationResult Calc_Multiply_Operation_Handler(void* context);
            static HALOperationResult Calc_Divide_Operation_Handler(void* context);
            static HALOperationResult Calc_Modulus_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseAnd_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseOr_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseExOr_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseLeftShift_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseRightShift_Operation_Handler(void* context);

            static HALOperationResult Compare_NotEqualsTo_Operation_Handler(void* context);
            static HALOperationResult Compare_EqualsTo_Operation_Handler(void* context);
            static HALOperationResult Compare_LessThan_Operation_Handler(void* context);
            static HALOperationResult Compare_GreaterThan_Operation_Handler(void* context);
            static HALOperationResult Compare_LessThanOrEqual_Operation_Handler(void* context);
            static HALOperationResult Compare_GreaterThanOrEqual_Operation_Handler(void* context);

            static RPNHandler GetRPN_OperatorFunction(ExpTokenType type);

            CalcRPNToken();
            ~CalcRPNToken();
        };

        struct CalcRPN {
            CalcRPNToken* items;
            int count;
            
            CalcRPN(CalcRPN&) = delete;
            CalcRPN(ExpressionTokens* tokens, int startIndex, int endIndex);

            HALOperationResult DoCalc();
        };
    }
}