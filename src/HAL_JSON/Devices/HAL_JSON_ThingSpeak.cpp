
#include "HAL_JSON_ThingSpeak.h"

namespace HAL_JSON {

    const char ThingSpeak::TS_ROOT_URL[] = "http://api.thingspeak.com/update?api_key=";
    
    ThingSpeak::ThingSpeak(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool ThingSpeak::VerifyJSON(const JsonVariant &jsonObj) {

        return true;
    }

    Device* ThingSpeak::Create(const JsonVariant &jsonObj, const char* type) {
        return new ThingSpeak(jsonObj, type);
    }

    String ThingSpeak::ToString() {
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