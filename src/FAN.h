
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

namespace FAN {

    uint8_t pin = 15;
    uint32_t freq = 25000;
    uint32_t value = 64;
    uint8_t resolution = 8;
    uint8_t inv_out = 1;

    uint32_t getInvValue(uint32_t val)
    {
        if (resolution == 10)
            return 1023-value;
        else if (resolution == 8)
            return 255-value;
        
        return value;
    }

    void init()
    {
        analogWriteResolution(resolution);
        analogWriteFreq(freq);
        if (inv_out)
            value = getInvValue(value);
        analogWrite(pin, value);
        
    }
    
    void SetFanSpeed(uint32_t val)
    {
        if (inv_out)
            value = getInvValue(val);
        else
            value = val;
        analogWrite(pin, val);
    }

    void DecodeFromJSON(JsonVariant &json)
    {
        if (json.containsKey("pin")) {
            pin = json["pin"];
        }
        if (json.containsKey("freq")) {
            freq = json["freq"];
            analogWriteFreq(freq);
        }
        if (json.containsKey("bits")) {
            resolution = json["bits"];
            analogWriteResolution(resolution);
        }
        if (json.containsKey("val")) {
            SetFanSpeed(json["val"]);
        }
    }
}