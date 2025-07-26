

#include "HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {
    CachedDeviceAccess::CachedDeviceAccess(const char* uidPath, const char* funcName) : path(uidPath) {
        device = Manager::findDevice(path);
        currentVersion = Manager::ReloadVersionPtr();
        cachedVersion = *currentVersion;
        if (funcName != nullptr && device != nullptr)
            readToHalValueFunc = device->GetReadToHALValue_Function(funcName);
        else
            readToHalValueFunc = nullptr;
    }

    Device* CachedDeviceAccess::GetDevice() {
        if (cachedVersion != *currentVersion) {
            device = Manager::findDevice(path);
            cachedVersion = *currentVersion; 
        }
        return device;
    }
}