#include "HAL_JSON_Device.h"

namespace HAL_JSON {

    Device::Device() {}

    Device::~Device() {}

    void Device::loop() {}
    Device* Device::findDevice(const UIDPath& path) { return nullptr; }

    String Device::ToString() {
        return "uid=" + String(decodeUID(uid).c_str());
    }
    bool Device::read(const HALReadRequest &req) { return false; }
    bool Device::write(const HALWriteRequest &req) { return false; };
    bool Device::read(const HALReadStringRequest &req) { return false; }
    bool Device::write(const HALWriteStringRequest &req) { return false; }

} // namespace HAL
