
#include "HAL_JSON_Display_SSD1306.h"

#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "../../../Support/CharArrayHelpers.h"

namespace HAL_JSON {

    Device* Display_SSD1306::Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire) {
        return new Display_SSD1306(jsonObj, type, wire);
    }
    
    Display_SSD1306::Display_SSD1306(const JsonVariant &jsonObj, const char* type, TwoWire& wire) : Device(UIDPathMaxLength::One,type) {

        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);

        uint32_t width = GetAsUINT32(jsonObj, "width");
        uint32_t height = GetAsUINT32(jsonObj, "height");
        const char* addrStr = GetAsConstChar(jsonObj, "addr");
        uint8_t addr = static_cast<uint8_t>(std::strtoul(addrStr, nullptr, 16));
        uint8_t textSize = GetAsUINT8(jsonObj, "textsize");
        if (textSize == 0) textSize = 1;

        display = new Adafruit_SSD1306(width, height, &Wire, -1); // -1 = no reset pin
        delay(200);
        if (display->begin(SSD1306_SWITCHCAPVCC, addr))
        {
            //delay(2000);
            //display->begin(SSD1306_SWITCHCAPVCC, addr);
            //delay(2000);
            display->clearDisplay();
            display->display();
            //display.setFont(&FreeMono9pt7b);
            display->setTextSize(textSize);
            display->setTextColor(WHITE, BLACK);
        }

        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();

        int itemCount = items.size();
        bool* validItems = new bool[itemCount];
        // first pass count valid items
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) { validItems[i] = false; continue; }// comment item
            if (Device::DisabledInJson(item) == true) { validItems[i] = false; continue; } // disabled

            if (Display_SSD1306_Element::VerifyJSON(item) == false) { validItems[i] = false; continue; }
            validItemCount++;
            validItems[i] = true;
        }
        // second pass actually create the devices
        elementCount = validItemCount;
        elements = new Display_SSD1306_Element*[validItemCount];
        int index = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (validItems[i] == false) continue;
            
            elements[index++] = new Display_SSD1306_Element(item, "I2C_DISP_SSD1306_ELM");
        }
        delete[] validItems;
    }

    Display_SSD1306::~Display_SSD1306() {
        if (elements != nullptr) {
            for (int i=0;i<elementCount;i++) {
                delete elements[i];
                elements[i] = nullptr;
            }
            delete[] elements;
            elements = nullptr;
            elementCount = 0;
        }
        delete display;
    }

    bool Display_SSD1306::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_1WTD_VERIFY_JSON); return false; }

        bool anyError = false == GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sckpin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
        anyError = false == GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sdapin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN));
        anyError = false == ValidateUINT32(jsonObj, "width");
        anyError = false == ValidateUINT32(jsonObj, "height");
        anyError = false == ValidateJsonStringField(jsonObj, "addr");
        

        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("Display_SSD1306"));
            return false;
        }
        int itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            
            if (Display_SSD1306_Element::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;

        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("Display_SSD1306"));
            return false;
        }

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