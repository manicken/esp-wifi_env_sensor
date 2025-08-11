#include "HAL_JSON_Device.h"

namespace HAL_JSON {

    Device::Device(UIDPathMaxLength uidMaxLength, const char* type) : uidMaxLength(static_cast<uint8_t>(uidMaxLength)), type(type) { }

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

    HALOperationResult  Device::read(HALValue& val) { return HALOperationResult ::UnsupportedOperation; }
    HALOperationResult  Device::write(const HALValue& val) { return HALOperationResult ::UnsupportedOperation; };
    HALOperationResult  Device::read(const HALReadStringRequestValue& val) { return HALOperationResult ::UnsupportedOperation; }
    HALOperationResult  Device::write(const HALWriteStringRequestValue& val) { return HALOperationResult ::UnsupportedOperation; }
    HALOperationResult  Device::read(const HALReadValueByCmd& val) { return HALOperationResult ::UnsupportedOperation; }
    HALOperationResult  Device::write(const HALWriteValueByCmd& val) { return HALOperationResult ::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType Device::GetReadToHALValue_Function(const char* funcName) { return nullptr; }
    HALValue* Device::GetValueDirectAccessPtr() { return nullptr; }

    bool Device::DisabledInJson(const JsonVariant& jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_DISABLED) == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_DISABLED].is<bool>() == false) return false;
        return jsonObj[HAL_JSON_KEYNAME_DISABLED].as<bool>(); 
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
