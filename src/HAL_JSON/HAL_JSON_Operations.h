
#pragma once

#ifdef _WIN32

#include <cstdint>
//using String = std::string;
#endif

#include <string>

#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {
    struct HALWriteValueByCmd {
        const HALValue& value;
        const std::string& cmd;
        HALWriteValueByCmd(const HALValue& value, const std::string& cmd): value(value), cmd(cmd) {}
    };
    struct HALWriteValueByCmdReq {
        UIDPath& path;
        const HALWriteValueByCmd& valByCmd;
        HALWriteValueByCmdReq(UIDPath& p, const HALWriteValueByCmd& valByCmd) : path(p), valByCmd(valByCmd) {}
    };

    struct HALReadValueByCmd {
        HALValue& out_value;
        const std::string& cmd;
        HALReadValueByCmd(HALValue& out, const std::string& cmd): out_value(out), cmd(cmd) {}
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
        const std::string& cmd;
        HALReadStringRequestValue(const std::string& cmd, std::string& out_value): out_value(out_value), cmd(cmd) {}
    };

    struct HALWriteStringRequestValue {
        const std::string& value;
        std::string& result;
        HALWriteStringRequestValue(const std::string& value, std::string& result): value(value), result(result) {}
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