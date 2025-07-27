#ifndef TICKER_H
#define TICKER_H

#include <Arduino.h>
#include <functional>
#include <thread>
#include <chrono>

class Ticker {
public:
    template<typename TFunc, typename... TArgs>
    void once_ms(unsigned long delayMs, TFunc func, TArgs... args) {
        std::thread([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            func(args...);
        }).detach(); // Detached thread simulates async behavior
    }

    void detach() {
        // No-op for simulation
    }
};

#endif // TICKER_H
