
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

    struct HALReadStringRequest {
        const UIDPath& path;
        String& out_value;
        HALReadStringRequest(const UIDPath& p, String& out) : path(p), out_value(out) {}
    };

    struct HALWriteStringRequest {
        const UIDPath& path;
        const String& value;
        HALWriteStringRequest(const UIDPath& p, const String& val) : path(p), value(val) {}
    };
}