

#include "HAL_JSON_CachedDevicePath.h"

namespace HAL_JSON {
    CachedDevicePath::CachedDevicePath(const char* uidPath) : path(uidPath) {
        device = Manager::findDevice(path);
        cachedVersion = Manager::ReloadVersion();
    }

    Device* CachedDevicePath::GetDevice() {
        if (cachedVersion != Manager::ReloadVersion()) {
            device = Manager::findDevice(path);
            cachedVersion = Manager::ReloadVersion();
        }
        return device;
    }
}