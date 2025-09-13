
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"

namespace HAL_JSON {

    class Display_SSD1306_Element : public Device {
    private:
        
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);

        Display_SSD1306_Element(const JsonVariant &jsonObj, const char* type);

        String ToString() override;
    };
}