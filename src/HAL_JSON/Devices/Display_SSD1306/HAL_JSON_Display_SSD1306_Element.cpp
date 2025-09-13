
#include "HAL_JSON_Display_SSD1306_Element.h"

namespace HAL_JSON {
    
    HAL_JSON_Display_SSD1306_Element::HAL_JSON_Display_SSD1306_Element(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool HAL_JSON_Display_SSD1306_Element::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    String HAL_JSON_Display_SSD1306_Element::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        return ret;
    }

}