
#pragma once

#include <Arduino.h>

#include <string>

#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {
    struct HALWriteValueByCmd {
        const HALValue& value;
        //const std::string& cmd;
        const ZeroCopyString& cmd;
        HALWriteValueByCmd(const HALValue& value, const ZeroCopyString& cmd): value(value), cmd(cmd) {}
    };
    struct HALWriteValueByCmdReq {
        UIDPath& path;
        const HALWriteValueByCmd& valByCmd;
        HALWriteValueByCmdReq(UIDPath& p, const HALWriteValueByCmd& valByCmd) : path(p), valByCmd(valByCmd) {}
    };
    enum class HALOperationResult {
        Success = 0,
        DeviceNotFound = 1,
        UnsupportedOperation = 2,  // was OperationNotSupported
        UnsupportedCommand = 3,   // was InvalidCommand
        ExecutionFailed = 4,       // was OperationFail

        /** script engine specific error */
        StackUnderflow = 10,
        /** script engine specific error */
        StackOverflow = 11,
        /** script engine specific error */
        DivideByZero = 12
    };
    inline const char* ToString(HALOperationResult result) {
        switch (result) {
            case HALOperationResult::Success: return "Success";
            case HALOperationResult::DeviceNotFound: return "DeviceNotFound";
            case HALOperationResult::UnsupportedOperation: return "UnsupportedOperation";
            case HALOperationResult::UnsupportedCommand: return "UnsupportedCommand";
            case HALOperationResult::ExecutionFailed: return "ExecutionFailed";
            default: return "Unknown";
        }
    }
    struct HALReadValueByCmd {
        HALValue& out_value;
        //const std::string& cmd;
        const ZeroCopyString& cmd;
        HALReadValueByCmd(HALValue& out, const ZeroCopyString& cmd): out_value(out), cmd(cmd) {}
    };

    struct HALReadValueByCmdReq {
        UIDPath& path;
        const HALReadValueByCmd& valByCmd;
        HALReadValueByCmdReq(UIDPath& p, const HALReadValueByCmd& valByCmd) : path(p), valByCmd(valByCmd) {}
    };

    struct HALReadRequest {
        UIDPath& path;
        HALValue& out_value;
        HALReadRequest(UIDPath& p, HALValue& out) : path(p), out_value(out) {}
    };

    struct HALWriteRequest {
        UIDPath& path;
        const HALValue& value;
        HALWriteRequest(UIDPath& p, const HALValue& val) : path(p), value(val) {}
    };

    struct HALReadStringRequestValue {
        std::string& out_value;
        //const std::string& cmd;
        const ZeroCopyString& cmd;
        HALReadStringRequestValue(const ZeroCopyString& cmd, std::string& out_value): out_value(out_value), cmd(cmd) {}
    };

    struct HALWriteStringRequestValue {
        //const std::string& value;
        const ZeroCopyString& value;
        std::string& result;
        HALWriteStringRequestValue(const ZeroCopyString& value, std::string& result): value(value), result(result) {}
    };

    struct HALReadStringRequest {
        UIDPath& path;
        HALReadStringRequestValue& value;
        HALReadStringRequest(UIDPath& p, HALReadStringRequestValue& value) : path(p), value(value) {}
    };

    struct HALWriteStringRequest {
        UIDPath& path;
        HALWriteStringRequestValue& value;
        HALWriteStringRequest(UIDPath& p, HALWriteStringRequestValue& value) : path(p), value(value) {}
    };
}