
#include "HAL_JSON_REGO600.h"

namespace HAL_JSON {
    
    REGO600::REGO600(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool REGO600::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    Device* REGO600::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600(jsonObj, type);
    }

    String REGO600::ToString() {
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