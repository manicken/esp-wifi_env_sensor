
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_Device.h" // HALOperationResult
#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_CalcRPNToken.h"
#include "../HAL_JSON_CachedDeviceAccess.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"


namespace HAL_JSON {
    namespace ScriptEngine {

        using ActionHandler = HALOperationResult(*)(void*);

        struct ActionStatement
        {
            HAL_JSON_NOCOPY_NOMOVE(ActionStatement);

            CachedDeviceAccess* target;
            CalcRPN* calcRpn;

            ActionStatement(Tokens& tokens, ActionHandler& handlerOut);
            ~ActionStatement();

            
            template <typename Op>
            static HALOperationResult CompoundAssign_Handler(void* context) {
                ActionStatement* actionItem = static_cast<ActionStatement*>(context);
                HALOperationResult res = actionItem->calcRpn->DoCalc();
                if (res != HALOperationResult::Success) return res;
                HALValue val2write;
                if (!halValueStack.GetFinalResult(val2write)) return HALOperationResult::ResultGetFail;
                HALValue readVal;
                res = actionItem->target->ReadSimple(readVal);
                if (res != HALOperationResult::Success) return res;
                val2write = Op::apply(readVal, val2write);
                return actionItem->target->WriteSimple(val2write);
            }
            

            static HALOperationResult Assign_Handler(void* context);
/*            static HALOperationResult AddAndAssign_Handler(void* context);
            static HALOperationResult SubtractAndAssign_Handler(void* context);
            static HALOperationResult MultiplyAndAssign_Handler(void* context);
            static HALOperationResult DivideAndAssign_Handler(void* context);
            static HALOperationResult ModulusAssign_Handler(void* context);
            static HALOperationResult BitwiseOrAssign_Handler(void* context);
            static HALOperationResult BitwiseAndAssign_Handler(void* context);
            static HALOperationResult BitwiseExOrAssign_Handler(void* context);
            static HALOperationResult BitwiseShiftRightAssign_Handler(void* context);
            static HALOperationResult BitwiseShiftLeftAssign_Handler(void* context);
            */
            static ActionHandler GetFunctionHandler(const char c);

            
        };

        

    }
}