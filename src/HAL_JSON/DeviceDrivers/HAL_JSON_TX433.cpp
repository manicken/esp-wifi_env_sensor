
#include "HAL_JSON_TX433.h"

namespace HAL_JSON {
    
    TX433::TX433(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool TX433::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT)));
    }

    Device* TX433::Create(const JsonVariant &jsonObj, const char* type) {
        return new TX433(jsonObj, type);
    }

    Device* TX433::findDevice(UIDPath& path) {
        return nullptr;
    }

    bool TX433::write(const HALValue &val) {
        return false;
    }

    bool TX433::write(const HALWriteStringRequestValue &val) {
        return false;
    }

    String TX433::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += pin;
        return ret;
    }

}