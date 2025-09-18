
#include "HAL_JSON_template.h"

namespace HAL_JSON {
    
    Template::Template(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool Template::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    Device* Template::Create(const JsonVariant &jsonObj, const char* type) {
        return new Template(jsonObj, type);
    }

    String Template::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        return ret;
    }

    void Template::loop() {}
    void Template::begin() {}
    Device* Template::findDevice(UIDPath& path) { return nullptr; }

    HALOperationResult Template::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::write(const HALValue& val) { return HALOperationResult::UnsupportedOperation; };
    HALOperationResult Template::read(const HALReadStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::write(const HALWriteStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::read(const HALReadValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::write(const HALWriteValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::exec() { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::exec(ZeroCopyString& cmd) { return HALOperationResult::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType Template::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::ReadToHALValue_FuncType Template::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* Template::GetValueDirectAccessPtr() { return nullptr; }
}