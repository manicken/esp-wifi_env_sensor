
#include "HAL_JSON_Display_SSD1306_Element.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {
    
    Display_SSD1306_Element::Display_SSD1306_Element(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        xPos = GetAsUINT8(jsonObj, "x");
        yPos = GetAsUINT8(jsonObj, "y");
        const char* labelStr = GetAsConstChar(jsonObj,"label");
        label = labelStr;

        const char* sourceStr = GetAsConstChar(jsonObj,"source");
        if (sourceStr != nullptr) {
            ZeroCopyString zcFuncName(sourceStr);
            ZeroCopyString zcSource = zcFuncName.SplitOffHead('#');
             
            cdaSource = new CachedDeviceAccess(zcSource, zcFuncName);
        }
        else
            cdaSource = nullptr;
    }

    bool Display_SSD1306_Element::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_ELM_VERIFY_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj, "label")){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_ELM_VERIFY_JSON); return false; }
        if (ValidateUINT8(jsonObj, "x") == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_ELM_VERIFY_JSON); return false;  }
        if (ValidateUINT8(jsonObj, "y") == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_ELM_VERIFY_JSON); return false;  }
        return true;
    }

    String Display_SSD1306_Element::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        return ret;
    }

    HALOperationResult Display_SSD1306_Element::write(const HALValue& val) {
        this->val = val;
        return HALOperationResult::Success;
    }

}