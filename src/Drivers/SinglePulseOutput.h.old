#ifndef SINGLE_PULSE_OUTPUT_H
#define SINGLE_PULSE_OUTPUT_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

class SinglePulseOutput {
public:
    SinglePulseOutput(uint8_t _pin, uint8_t _inactiveState = LOW);
    ~SinglePulseOutput();
    void pulse(unsigned long durationMs);

private:
    uint8_t pin = 0;
    uint8_t inactiveState = LOW;
    TimerHandle_t timer = NULL;
    void init();
    static void timerCallbackStatic(TimerHandle_t xTimer);
    void endPulse();
};

#endif
