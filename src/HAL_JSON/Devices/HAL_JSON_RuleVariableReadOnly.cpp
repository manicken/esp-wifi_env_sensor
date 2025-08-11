
#include "HAL_JSON_RuleVariableReadOnly.h"

namespace HAL_JSON {
    
    RuleVariableReadOnly::RuleVariableReadOnly(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
    }

    bool RuleVariableReadOnly::VerifyJSON(const JsonVariant &jsonObj) {
        // could add type def later if wanted
        // also nonvolatile storage could be a mode
        return true;
    }

    Device* RuleVariableReadOnly::Create(const JsonVariant &jsonObj, const char* type) {
        return new RuleVariableReadOnly(jsonObj, type);
    }

    HALOperationResult  RuleVariableReadOnly::read(HALValue& val) {
        val = value;
        return HALOperationResult::Success;
    }

    String RuleVariableReadOnly::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += DeviceConstStrings::value;//StartWithComma;
        if (value.getType() == HALValue::Type::FLOAT)
            ret += std::to_string(value.asFloat()).c_str();
        else if (value.getType() == HALValue::Type::UINT)
            ret += std::to_string(value.asUInt()).c_str();
        else if (value.getType() == HALValue::Type::INT)
            ret += std::to_string(value.asInt()).c_str();
        else
            ret += "\"not set\"";
        return ret;
    }

}