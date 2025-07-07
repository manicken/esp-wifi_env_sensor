
#include "HAL_JSON_TX433unit.h"

namespace HAL_JSON {
    
    TX433unit::TX433unit(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool TX433unit::VerifyJSON(const JsonVariant &jsonObj) {
        return false;
    }

    Device* TX433unit::findDevice(UIDPath& path) {
        return nullptr;
    }

    bool TX433unit::write(const HALValue &val) {
        return false;
    }

    bool TX433unit::write(const HALWriteStringRequestValue &val) {
        return false;
    }

    String TX433unit::ToString() {
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