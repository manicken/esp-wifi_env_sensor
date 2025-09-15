
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {

    class Display_SSD1306_Element : public Device {
        
    public:
        CachedDeviceAccess* cdaSource;
        HALValue val;
        std::string label;
        uint8_t xPos;
        uint8_t yPos;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        Display_SSD1306_Element(Display_SSD1306_Element&) = delete;
        Display_SSD1306_Element(const JsonVariant &jsonObj, const char* type);

        HALOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
/* the following is just to make Display_SSD1306_Element more streamlined without using cdaSource
    however as the DisplayElements would not be so many, it maybe not matter
    class Display_SSD_ElementFromSource : public Display_SSD1306_Element {
    private:
        
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        Display_SSD_ElementFromSource(const JsonVariant &jsonObj, const char* type);
    };
    */
}