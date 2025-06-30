
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"

// DHT models
#define HAL_JSON_TYPE_DHT_MODEL_DHT11       "DHT11"
#define HAL_JSON_TYPE_DHT_MODEL_DHT22       "DHT22"
#define HAL_JSON_TYPE_DHT_MODEL_AM2302      "AM2302"
#define HAL_JSON_TYPE_DHT_MODEL_RHT03       "RTH03"

namespace HAL_JSON {

    class DHT : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        DHT(const JsonVariant &jsonObj, const char* type);

        String ToString() override;

        void loop() override; // will need loop for automatic polling as this device is slow
    };

}