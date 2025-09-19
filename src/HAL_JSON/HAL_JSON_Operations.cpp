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

#include "HAL_JSON_Operations.h"

namespace HAL_JSON {
    const char* HALOperationResultToString(HALOperationResult result) {
        switch (result) {
            case HALOperationResult::Success: return "Success";
            case HALOperationResult::DeviceNotFound: return "DeviceNotFound";
            case HALOperationResult::UnsupportedOperation: return "UnsupportedOperation";
            case HALOperationResult::UnsupportedCommand: return "UnsupportedCommand";
            case HALOperationResult::ExecutionFailed: return "ExecutionFailed";
            case HALOperationResult::IfConditionTrue: return "IfConditionTrue";
            case HALOperationResult::IfConditionFalse: return "IfConditionFalse";
            case HALOperationResult::StackUnderflow: return "StackUnderflow";
            case HALOperationResult::DivideByZero: return "DivideByZero";
            case HALOperationResult::StackOverflow: return "StackOverflow";
            case HALOperationResult::ResultGetFail: return "ResultGetFail";
            case HALOperationResult::HandlerWasNullPtr: return "HandlerWasNullPtr";
            case HALOperationResult::ContextWasNullPtr: return "ContextWasNullPtr";
            case HALOperationResult::StringRequestParameterError: return "StringRequestParameterError";
            case HALOperationResult::NotSet: return "NotSet";
            
            default:
                printf("unknown HALOperationResult: %d\n", (int)result);
                return "Unknown";
        }
    }
}