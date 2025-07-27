#pragma once

#include <Arduino.h>

class OneWire {
public:
    OneWire(uint8_t pin);
    void reset_search();
    bool search(uint8_t *newAddr, bool search_mode = true);
};