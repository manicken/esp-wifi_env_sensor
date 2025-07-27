
#include "DallasTemperature.h"


DallasTemperature::DallasTemperature(OneWire* _oneWire) {}
void DallasTemperature::setWaitForConversion(bool val) {}
void DallasTemperature::requestTemperatures() {}
float DallasTemperature::getTempC(const uint8_t* deviceAddress) { return 0.0f; }
float DallasTemperature::getTempF(const uint8_t* deviceAddress) { return 0.0f; }