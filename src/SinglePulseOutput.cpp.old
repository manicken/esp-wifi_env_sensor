#include "SinglePulseOutput.h"

SinglePulseOutput::SinglePulseOutput(uint8_t _pin, uint8_t _inactiveState) 
    : pin(_pin), inactiveState(_inactiveState) {

    pinMode(pin, OUTPUT);
    digitalWrite(pin, inactiveState);

    timer = xTimerCreate(
        "ReusablePulseTimer",
        pdMS_TO_TICKS(1000),  // Default duration, will update dynamically
        pdFALSE,
        this,
        timerCallbackStatic
    );
}

void SinglePulseOutput::pulse(unsigned long durationMs) {
    digitalWrite(pin, !inactiveState);

    if (timer != NULL) {
        // Update timer period
        xTimerStop(timer, 0);
        xTimerChangePeriod(timer, pdMS_TO_TICKS(durationMs), 0);
        xTimerStart(timer, 0);
    }
}

void SinglePulseOutput::timerCallbackStatic(TimerHandle_t xTimer) {
    SinglePulseOutput* instance = static_cast<SinglePulseOutput*>(pvTimerGetTimerID(xTimer));
    if (instance) {
        instance->endPulse();
    }
}

void SinglePulseOutput::endPulse() {
    digitalWrite(pin, inactiveState);
}

SinglePulseOutput::~SinglePulseOutput() {
    if (timer != nullptr) {
        xTimerDelete(timer, 0);
        timer = nullptr;
    }
}
