#pragma once

#include <Arduino.h>
#include <OneWire.h>

class DallasTemperature {
public:
    DallasTemperature(OneWire* _oneWire);
    void setWaitForConversion(bool val);
    void requestTemperatures();
    float getTempC(const uint8_t* deviceAddress);
    float getTempF(const uint8_t* deviceAddress);
};