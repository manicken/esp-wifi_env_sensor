
#include "HAL_JSON_Display_SSD1306_Element.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {
    
    Display_SSD1306_Element::Display_SSD1306_Element(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);

    }

    bool Display_SSD1306_Element::VerifyJSON(const JsonVariant &jsonObj) {
         if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_1WTD_VERIFY_JSON); return false; }
         
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

}