#include "HALDevice.h"

namespace HAL {

    Device::Device() {}

    Device::~Device() {}

    void Device::loop() {}

    String Device::ToString() {
        return "uid=" + String(decodeUID(uid).c_str());
    }

    bool Device::read(String &val) { return false; }
    bool Device::write(String val) { return false; }

} // namespace HAL
