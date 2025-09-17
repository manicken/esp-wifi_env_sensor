
#include "HAL_JSON_PCF8574x.h"

namespace HAL_JSON {
    
    PCF8574x::PCF8574x(const JsonVariant &jsonObj, const char* type, TwoWire& wire) : Device(UIDPathMaxLength::One,type), wire(&wire) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        const char* addrStr = GetAsConstChar(jsonObj, "addr");
        addr = static_cast<uint8_t>(std::strtoul(addrStr, nullptr, 16));

    }

    bool PCF8574x::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ 
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        if (false == ValidateJsonStringField(jsonObj, "addr")) {
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        return true;
    }

    Device* PCF8574x::Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire) {
        return new PCF8574x(jsonObj, type, wire);
    }

    HALOperationResult PCF8574x::read(HALValue& val) {
        uint8_t received = wire->requestFrom(addr, (uint8_t)1);
        if (received == 0) return HALOperationResult::ExecutionFailed;
        val = (uint32_t)wire->read();
        return HALOperationResult::Success;
    }
    HALOperationResult PCF8574x::write(const HALValue& val) {
        wire->beginTransmission(addr);
        wire->write(val.asUInt());
        uint8_t res = wire->endTransmission(true);
        if (res != 0) {
            // todo maybe log to global logger
            return HALOperationResult::ExecutionFailed;
        }
        return HALOperationResult::Success;
    }

    String PCF8574x::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += "\"addr\":\"0x";
        ret += Convert::toHex(addr).c_str();
        ret += "\"";
        return ret;
    }

}