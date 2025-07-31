
#include "Arduino.h"
#include <iostream>

SerialStub Serial;
//uint64_t lastmicros = 0;
void digitalWrite(int pin, int val) {
    //using namespace std::chrono;
    //auto now = high_resolution_clock::now();
    //uint64_t micros = duration_cast<microseconds>(now.time_since_epoch()).count();

    //std::cout << "[" << micros-lastmicros << " uS] digitalWrite(" << pin << ", " << val << ")\n";
    std::cout << "digitalWrite(" << pin << ", " << val << ")\n";
    //lastmicros = micros;
}