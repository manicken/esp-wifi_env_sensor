#pragma once

#include <Arduino.h> // Needed for String class


#include <OneWire.h>
#include <DallasTemperature.h>

#include <ArduinoJson.h>

#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "HAL_JSON_OneWireTempAutoRefresh.h"

#define HAL_JSON_KEYNAME_ONE_WIRE_ROMID       "romid"
#define HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT "format"

namespace HAL_JSON {

    enum class OneWireTempDeviceTempFormat {
        Celsius,
        Fahrenheit
    };

    typedef struct {
        union {
            uint8_t bytes[8];
            uint64_t id;
        };
    } OneWireAddress;

    class OneWireTempDevice : public Device {
    public:
        OneWireAddress romid;
        OneWireTempDeviceTempFormat format = OneWireTempDeviceTempFormat::Celsius;
        float value = 0.0f;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        OneWireTempDevice(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempDevice();
        
        HALOperationResult read(HALValue& val) override;

        String ToString() override;
    };

    class OneWireTempDeviceAtRoot : public OneWireTempDevice {
    private:
        OneWireTempAutoRefresh autoRefresh;
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
        void requestTemperatures();
        void readAll();
    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        
        OneWireTempDeviceAtRoot(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempDeviceAtRoot();
//#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        HALOperationResult write(const HALValue& val) override;
//#endif
        void loop() override;

        String ToString() override;
    };
}