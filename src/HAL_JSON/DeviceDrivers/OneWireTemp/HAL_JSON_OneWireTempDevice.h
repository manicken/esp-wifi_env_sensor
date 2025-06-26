#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_DeviceTypeDefNames.h"
#include "../../ArduinoJSON_ext.h"
#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    enum class OneWireTempDeviceTempFormat {
        Celsius,
        Fahrenheit
    };

    class OneWireTempDevice : public Device {
    public:
        uint8_t romid[8];
        OneWireTempDeviceTempFormat format = OneWireTempDeviceTempFormat::Celsius;
        float value;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        OneWireTempDevice(const JsonVariant &jsonObj);
        ~OneWireTempDevice();
        
        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest&req) override;
        String ToString();
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
        static Device* Create(const JsonVariant &jsonObj);
        
        OneWireTempDeviceAtRoot(const JsonVariant &jsonObj);
        ~OneWireTempDeviceAtRoot();

        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest&req) override;
        String ToString();
    };
}