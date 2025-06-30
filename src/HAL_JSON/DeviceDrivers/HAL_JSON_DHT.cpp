
#include "HAL_JSON_DHT.h"

namespace HAL_JSON {

    DHT::DHT(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {

    }

    bool DHT::VerifyJSON(const JsonVariant &jsonObj) {
        return false;
    }

    Device* DHT::Create(const JsonVariant &jsonObj, const char* type) {
        return new DHT(jsonObj, type);
    }

    String DHT::ToString() {
        String ret;
        ret += "\"type\":\"" +String(type)+ "\"";
        ret += ",\"pin\":" + String(pin);
        return ret;
    }

    void DHT::loop() {

    }
    
}