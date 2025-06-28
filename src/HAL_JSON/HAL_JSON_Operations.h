
#pragma once
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {

    struct HALReadRequest {
        const UIDPath& path;
        HALValue& out_value;
        HALReadRequest(const UIDPath& p, HALValue& out) : path(p), out_value(out) {}
    };

    struct HALWriteRequest {
        const UIDPath& path;
        const HALValue& value;
        HALWriteRequest(const UIDPath& p, const HALValue& val) : path(p), value(val) {}
    };

    struct HALReadStringRequestValue {
        String& out_value;
        const String& cmd;
        HALReadStringRequestValue(String& out_value, const String& cmd): out_value(out_value), cmd(cmd) {}
    };

    struct HALWriteStringRequestValue {
        const String& value;
        String& result;
        HALWriteStringRequestValue(const String& value, String& result): value(value), result(result) {}
    };

    struct HALReadStringRequest {
        const UIDPath& path;
        HALReadStringRequestValue& value;
        HALReadStringRequest(const UIDPath& p, HALReadStringRequestValue& value) : path(p), value(value) {}
    };

    struct HALWriteStringRequest {
        const UIDPath& path;
        HALWriteStringRequestValue& value;
        HALWriteStringRequest(const UIDPath& p, HALWriteStringRequestValue& value) : path(p), value(value) {}
    };
}