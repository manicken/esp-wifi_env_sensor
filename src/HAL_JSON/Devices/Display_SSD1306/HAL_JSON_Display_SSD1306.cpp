
#include "HAL_JSON_Display_SSD1306.h"

namespace HAL_JSON {

    Device* Display_SSD1306::Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire) {
        return new Display_SSD1306(jsonObj, type, wire);
    }
    
    Display_SSD1306::Display_SSD1306(const JsonVariant &jsonObj, const char* type, TwoWire& wire) : Device(UIDPathMaxLength::One,type) {
        //sckPin = GetAsUINT8(jsonObj, "sckpin");
       // sdaPin = GetAsUINT8(jsonObj, "sdapin");
        uint32_t width = GetAsUINT32(jsonObj, "width");
        uint32_t height = GetAsUINT32(jsonObj, "height");

        //Wire.begin(sdaPin, sckPin);

        display = new Adafruit_SSD1306(width, height, &Wire, -1); // -1 = no reset pin
    }

    Display_SSD1306::~Display_SSD1306() {

        delete display;
    }

    bool Display_SSD1306::VerifyJSON(const JsonVariant &jsonObj) {
        bool anyError = false == GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sckpin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
        anyError = false == GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sdapin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN));
        anyError = false == ValidateUINT32(jsonObj, "width");
        anyError = false == ValidateUINT32(jsonObj, "height");
        // this is a check only to verify that the pin cfg exist
        return anyError == false;
    }

    String Display_SSD1306::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        
        return ret;
    }

}