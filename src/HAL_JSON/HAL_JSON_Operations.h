
#pragma once

#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {
    struct HALReadValueByCmd {
        HALValue& out_value;
        const String& cmd;
        HALReadValueByCmd(HALValue& out, const String& cmd): out_value(out), cmd(cmd) {}
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
        String& out_value;
        const String& cmd;
        HALReadStringRequestValue(const String& cmd, String& out_value): out_value(out_value), cmd(cmd) {}
    };

    struct HALWriteStringRequestValue {
        const String& value;
        String& result;
        HALWriteStringRequestValue(const String& value, String& result): value(value), result(result) {}
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