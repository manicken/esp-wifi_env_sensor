#include "HAL_JSON_Device.h"

namespace HAL_JSON {

    Device::Device(UIDPathMaxLength uidMaxLength, const char* type) : uidMaxLength(uidMaxLength), type(type) { }

    Device::~Device() {}

    void Device::loop() {}
    void Device::begin() {}
    bool Device::LoopTaskDone() {
        if (loopTaskDone == false)
            return false;
        loopTaskDone = false;
        return true;
    }
    Device* Device::findDevice(UIDPath& path) { return nullptr; }

    String Device::ToString() { return ""; }

    HALOperationResult Device::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Device::write(const HALValue& val) { return HALOperationResult::UnsupportedOperation; };
    HALOperationResult Device::read(const HALReadStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Device::write(const HALWriteStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Device::read(const HALReadValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Device::write(const HALWriteValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType Device::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    HALValue* Device::GetValueDirectAccessPtr() { return nullptr; }

    bool Device::DisabledInJson(const JsonVariant& jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_DISABLED) == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_DISABLED].is<bool>() == false) return false;
        return jsonObj[HAL_JSON_KEYNAME_DISABLED].as<bool>(); 
    }

    Device* Device::findInArray(Device** devices, int deviceCount, UIDPath& path, Device* currentDevice) {
        if (!devices || deviceCount == 0) return nullptr;
        if (path.empty()) return nullptr;

        HAL_UID currUID;

        // Determine which UID to compare at this level
        if (currentDevice && currentDevice->uid.IsSet()) {
            currUID = path.getNextUID();   // advance for subdevice
        } else {
            currUID = path.getCurrentUID(); // root level or placeholder
        }

        if (currUID.Invalid()) return nullptr;

        Device* indirectMatch = nullptr;

        for (int i = 0; i < deviceCount; i++) {
            Device* dev = devices[i];
            if (!dev) continue;

            if (dev->uid == currUID) {
                if ((dev->uidMaxLength == UIDPathMaxLength::One) || path.isLast()) {
                    return dev;  // exact match
                } else {
                    // If a device matched the currUID but couldn't directly resolve the full path,
                    // attempt an indirect lookup via the matched device.
                    indirectMatch = dev->findDevice(path); // recurse into children
                    break; // No need to continue — currUID match is unique
                }
            } else if (dev->uid.NotSet() && !path.isLast()) { // this will only happen on devices where uidMaxLenght>1
                Device* d = dev->findDevice(path); // recurse into placeholder
                if (d) return d; // match allways return valid device
            }
        }
        if (indirectMatch == nullptr) {
            GlobalLogger.Error(F("could not find device: "),path.ToString().c_str());
            return nullptr;
        }
        return indirectMatch;
    }

    namespace DeviceConstStrings {
        HAL_JSON_DEVICE_CONST_STR_DEFINE(uid, "\"uid\":\"");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(type, "\"type\":\""); // type allways after uid
        
        HAL_JSON_DEVICE_CONST_STR_DEFINE(pin, ",\"pin\":");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(value, "\"value\":");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(valueStartWithComma, ",\"value\":");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(refreshTimeMs, ",\"refreshTimeMs\":");

    }

} // namespace HAL
