
#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>

#include "HAL_JSON_Display_SSD1306_Element.h"

// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace HAL_JSON {

    class Display_SSD1306 : public Device {
    private:
        Adafruit_SSD1306* display;

        Display_SSD1306_Element** elements; // TODO change this into plain array for minimal heap fragmentation
        int elementCount;
    public:
        // here we could implement functions for to use with spi interface as well

        static Device* Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        Display_SSD1306(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        ~Display_SSD1306();

        String ToString() override;
    };
}