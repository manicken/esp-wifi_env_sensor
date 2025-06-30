
#include "HAL_JSON_template.h";

namespace HAL_JSON {
    
    Template::Template(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool Template::VerifyJSON(const JsonVariant &jsonObj) {
        return false;
    }

    Device* Template::Create(const JsonVariant &jsonObj, const char* type) {
        return new Template(jsonObj, type);
    }

    String Template::ToString() {
        String ret;
        ret += "\"type\":\"" "template" "\"";
        ret += ",\"pin\":" + String(pin);
        return ret;
    }

}