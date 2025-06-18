#include "HAL_JSON_Device.h"

namespace HAL_JSON {

    Device::Device() {}

    Device::~Device() {}

    void Device::loop() {}
    Device* Device::findDevice(uint64_t uid) { return nullptr; }

    String Device::ToString() {
        return "uid=" + String(decodeUID(uid).c_str());
    }
    bool Device::read(HALValue &val) { return false; }
    bool Device::write(const HALValue &val) { return false; };
    bool Device::read(String &val) { return false; }
    bool Device::write(const String &val) { return false; }

} // namespace HAL
