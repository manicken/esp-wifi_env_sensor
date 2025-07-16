
#include "HAL_JSON_RuleVariable.h"

namespace HAL_JSON {
    
    RuleVariable::RuleVariable(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
    }

    bool RuleVariable::VerifyJSON(const JsonVariant &jsonObj) {
        // could add type def later if wanted
        // also nonvolatile storage could be a mode
        return true;
    }

    Device* RuleVariable::Create(const JsonVariant &jsonObj, const char* type) {
        return new RuleVariable(jsonObj, type);
    }

    bool RuleVariable::read(HALValue& val) {
        val = value;
        return true;
    }
    bool RuleVariable::write(const HALValue& val) {
        value = val;
        return true;
    }

    String RuleVariable::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += DeviceConstStrings::value;//StartWithComma;
        if (value.getType() == HALValue::Type::FLOAT)
            ret += value.asFloat();
        else if (value.getType() == HALValue::Type::UINT)
            ret += value.asUInt();
        else if (value.getType() == HALValue::Type::INT)
            ret += value.asInt();
        else
            ret += "\"not set\"";
        return ret;
    }

}