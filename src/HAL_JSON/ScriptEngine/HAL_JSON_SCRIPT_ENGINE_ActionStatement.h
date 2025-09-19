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