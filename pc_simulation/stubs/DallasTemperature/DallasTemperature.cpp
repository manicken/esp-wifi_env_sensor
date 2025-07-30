
#include "DallasTemperature.h"

float getRandomValue() {
    // Create a random device and use it to seed a generator
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister engine

    // Create a distribution in the range [10.0, 40.0)
    std::uniform_real_distribution<float> dist(10.0f, 40.0f);

    // Generate a random number
    float randomNumber = dist(gen);
    return randomNumber;
}

DallasTemperature::DallasTemperature(OneWire* _oneWire) {}
void DallasTemperature::setWaitForConversion(bool val) {}
void DallasTemperature::requestTemperatures() {}
void DallasTemperature::setTempC(float value) {
    tempC = value;
}
float DallasTemperature::getTempC(const uint8_t* deviceAddress) {
    return tempC;
}
float DallasTemperature::getTempF(const uint8_t* deviceAddress) {
    return tempC * 9.0 / 5.0 + 32.0;
}