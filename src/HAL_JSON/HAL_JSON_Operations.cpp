
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