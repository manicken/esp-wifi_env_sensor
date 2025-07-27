#pragma once

#include <Arduino.h>

struct TempAndHumidity {
  float temperature;
  float humidity;
};

class DHTesp {
public:
    typedef enum {
        AUTO_DETECT,
        DHT11,
        DHT22,
        AM2302,  // Packaged DHT22
        RHT03    // Equivalent to DHT22
    }
    DHT_MODEL_t;

    typedef enum {
        ERROR_NONE = 0,
        ERROR_TIMEOUT,
        ERROR_CHECKSUM
    }
    DHT_ERROR_t;

    DHTesp();

    void setup(uint8_t pin, DHT_MODEL_t model);
    TempAndHumidity getTempAndHumidity();
};