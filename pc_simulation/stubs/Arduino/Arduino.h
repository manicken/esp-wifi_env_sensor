#pragma once

#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <string>    // std::string for Windows
#include <cstring>
#include <cstdint>
#include "../Stream_WIN.h"

#define F(x) x


using String = std::string;


#define strdup _strdup
#define __FlashStringHelper char 




#define HIGH 1
#define LOW  0
#define INPUT 0
#define OUTPUT 1

// Simulate millis() using std::chrono
inline unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

// Simulate delay() using std::this_thread::sleep_for
inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Stub pinMode / digitalWrite / digitalRead
inline void pinMode(int pin, int mode) {
    printf("pinMode(%d, %d)\n", pin, mode);
}
inline void digitalWrite(int pin, int val) {
    printf("digitalWrite(%d, %d)\n", pin, val);
}
inline int digitalRead(int pin) {
    return 0; // always low
}
inline void analogWrite(int pin, float val) {
    printf("analogWrite(%d, %f)\n", pin, val);
}

// dont care that this say its error here


#include <iostream>

// Minimal stub for Serial-like behavior
class SerialStub : public Stream {
public:
    void begin(int) {} // No-op for simulation

    template <typename T>
    void print(const T& val) { std::cout << val; }

    template <typename T>
    void println(const T& val) { std::cout << val << std::endl; }

    void println() { std::cout << std::endl; }
};

extern SerialStub Serial; // Declared here, defined in a .cpp

#define HardwareSerial Stream
