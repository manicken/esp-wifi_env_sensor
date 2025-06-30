
#include "HAL_JSON_DHT.h"

namespace HAL_JSON {

    DHT::DHT(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {

    }

    bool DHT::VerifyJSON(const JsonVariant &jsonObj) {

    }

    Device* DHT::Create(const JsonVariant &jsonObj) {
        return new DHT(jsonObj);
    }

    String DHT::ToString() {
        String ret;
        ret += "\"type\":\"" HAL_JSON_TYPE_DHT "\"";
        ret += ",\"pin\":" + String(pin);
        return ret;
    }
    
}