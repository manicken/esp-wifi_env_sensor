
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"

// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace HAL_JSON {

    class Display_SSD1306 : public Device {
    private:
        Adafruit_SSD1306* display;
    public:
        static Device* Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        Display_SSD1306(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        ~Display_SSD1306();

        String ToString() override;
    };
}