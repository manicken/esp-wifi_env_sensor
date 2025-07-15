
#include "HAL_JSON_template.h"

namespace HAL_JSON {
    
    Template::Template(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool Template::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    Device* Template::Create(const JsonVariant &jsonObj, const char* type) {
        return new Template(jsonObj, type);
    }

    String Template::ToString() {
        String ret;
        ret += "\"type\":\"" "template" "\"";
        ret += ",\"pin\":";
        ret += String(pin);
        return ret;
    }

}