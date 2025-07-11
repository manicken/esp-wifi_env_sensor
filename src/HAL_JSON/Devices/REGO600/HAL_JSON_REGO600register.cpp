
#include "HAL_JSON_REGO600register.h"

namespace HAL_JSON {
    
    REGO600register::REGO600register(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

        
    }

    bool REGO600register::VerifyJSON(const JsonVariant &jsonObj) {

        return false;
    }

    Device* REGO600register::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600register(jsonObj, type);
    }

    String REGO600register::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        return ret;
    }

}