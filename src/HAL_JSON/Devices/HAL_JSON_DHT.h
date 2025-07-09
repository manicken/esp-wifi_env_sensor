
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHTesp.h>

#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

#define HAL_JSON_KEYNAME_DHT_MODEL          "model"
// DHT models
#define HAL_JSON_TYPE_DHT_MODEL_DHT11       "DHT11"
#define HAL_JSON_TYPE_DHT_MODEL_DHT22       "DHT22"
#define HAL_JSON_TYPE_DHT_MODEL_AM2302      "AM2302"
#define HAL_JSON_TYPE_DHT_MODEL_RHT03       "RTH03"
 /* set to 2 sec to be safe, this also defines the minimum refreshrate possible */
#define HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS 2000

namespace HAL_JSON {

    class DHT : public Device {
    private:
        DHTesp dht;
        uint8_t pin = 0;
        TempAndHumidity data;
        uint32_t refreshTimeMs = HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS;
        uint32_t lastUpdateMs = 0;
        static bool isValidDHTModel(const char* model);
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        DHT(const JsonVariant &jsonObj, const char* type);

        String ToString() override;
        
        void loop() override; // will need loop for automatic polling as this device is slow

        bool read(HALValue &val) override;
        bool read(const HALReadValueByCmd &val) override;
        bool read(const HALReadStringRequestValue &val) override;
    };

}