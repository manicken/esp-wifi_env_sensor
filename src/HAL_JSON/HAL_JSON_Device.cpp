#include "HAL_JSON_Device.h"

namespace HAL_JSON {

    Device::Device(UIDPathMaxLength uidMaxLength) : uidMaxLength(static_cast<uint8_t>(uidMaxLength)) { }

    Device::~Device() {}

    void Device::loop() {}
    Device* Device::findDevice(UIDPath& path) { return nullptr; }

    String Device::ToString() {
        return "uid=" + String(decodeUID(uid).c_str());
    }
    bool Device::read(HALValue& val) { return false; }
    bool Device::write(const HALValue& req) { return false; };
    bool Device::read(const HALReadStringRequestValue &val) { return false; }
    bool Device::write(const HALWriteStringRequestValue &val) { return false; }

    bool Device::DisabledInJson(const JsonVariant& jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_DISABLED) == false) return false;
        if (jsonObj.is<bool>() == false) return false;
        return jsonObj.as<bool>(); 
    }

} // namespace HAL
