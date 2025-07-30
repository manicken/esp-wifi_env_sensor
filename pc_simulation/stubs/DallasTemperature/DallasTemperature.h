#pragma once

#include <Arduino.h>
#include <OneWire.h>

#include <random>

class DallasTemperature {
private:
    float tempC = 20.0f;
public:
    DallasTemperature(OneWire* _oneWire);
    void setWaitForConversion(bool val);
    void requestTemperatures();
    float getTempC(const uint8_t* deviceAddress);
    float getTempF(const uint8_t* deviceAddress);
    void setTempC(float value);
};