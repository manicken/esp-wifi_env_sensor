

#include "HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {

    CachedDeviceAccess::CachedDeviceAccess(ZeroCopyString& uidPath, ZeroCopyString& funcName) : path(uidPath) {
        device = Manager::findDevice(path);
        currentVersion = Manager::ReloadVersionPtr();
        cachedVersion = *currentVersion;
        if (device == nullptr) {
            readToHalValueFunc = nullptr;
            valueDirectAccessPtr = nullptr;
            std::string uidStr = uidPath.ToString();
            printf("@CachedDeviceAccess const - device not found:>>%s<<\n", uidStr.c_str());
            return;
        }
        if (funcName != nullptr && device != nullptr) {
            readToHalValueFunc = device->GetReadToHALValue_Function(funcName);
            writeFromHalValueFunc = device->GetWriteFromHALValue_Function(funcName);
        }
        else {
            readToHalValueFunc = nullptr;
            writeFromHalValueFunc = nullptr;
        }
        
        valueDirectAccessPtr = device->GetValueDirectAccessPtr();
        
    }

    Device* CachedDeviceAccess::GetDevice() {
        if (cachedVersion != *currentVersion) {
            device = Manager::findDevice(path);
            cachedVersion = *currentVersion; 
        }
        return device;
    }

    HALOperationResult CachedDeviceAccess::WriteSimple(const HALValue& val) {
        if (writeFromHalValueFunc != nullptr) {
            Device* device = GetDevice();
            if (device == nullptr) return HALOperationResult::DeviceNotFound;
            HALValue valToWrite = val;
            //printf("\nWriteSimple - writeFromHalValueFunc\n");
            return writeFromHalValueFunc(device, valToWrite);
        }
        if (valueDirectAccessPtr != nullptr) {
            *valueDirectAccessPtr = val;
            return HALOperationResult::Success;
        }
        //printf("\nWriteSimple - device->write(val)\n");
        Device* device = GetDevice();
        if (device == nullptr) return HALOperationResult::DeviceNotFound;
        return device->write(val);
    }

    HALOperationResult CachedDeviceAccess::ReadSimple(HALValue& val) {
        if (readToHalValueFunc != nullptr) {
            Device* device = GetDevice();
            if (device == nullptr) return HALOperationResult::DeviceNotFound;
            return readToHalValueFunc(device, val);
        }
        if (valueDirectAccessPtr != nullptr) {
            val = *valueDirectAccessPtr;
            return HALOperationResult::Success;
        }
        Device* device = GetDevice();
        if (device == nullptr) return HALOperationResult::DeviceNotFound;
        return device->read(val);
    }
}