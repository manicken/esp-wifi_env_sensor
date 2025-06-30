
#include "HAL_JSON_template.h";

namespace HAL_JSON {
    
    Template::Template(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {

    }

    bool Template::VerifyJSON(const JsonVariant &jsonObj) {

    }

    Device* Template::Create(const JsonVariant &jsonObj) {
        return new Template(jsonObj);
    }

    String Template::ToString() {
        String ret;
        ret += "\"type\":\"" "template" "\"";
        ret += ",\"pin\":" + String(pin);
        return ret;
    }

}