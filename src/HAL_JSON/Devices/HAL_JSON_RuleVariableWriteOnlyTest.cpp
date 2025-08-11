
#include "HAL_JSON_RuleVariableWriteOnlyTest.h"

namespace HAL_JSON {
    
    RuleVariableWriteOnlyTest::RuleVariableWriteOnlyTest(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
    }

    bool RuleVariableWriteOnlyTest::VerifyJSON(const JsonVariant &jsonObj) {
        // could add type def later if wanted
        // also nonvolatile storage could be a mode
        return true;
    }

    Device* RuleVariableWriteOnlyTest::Create(const JsonVariant &jsonObj, const char* type) {
        return new RuleVariableWriteOnlyTest(jsonObj, type);
    }

    HALOperationResult  RuleVariableWriteOnlyTest::write(const HALValue& val) {
        value = val;
        return HALOperationResult::Success;
    }

    String RuleVariableWriteOnlyTest::ToString() {
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