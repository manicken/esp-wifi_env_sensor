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
#include "HAL_JSON_OneWireTempDevice.h"
#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    class OneWireTempBus : public Device {
    private:
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
    public:
        uint32_t deviceCount = 0;
        OneWireTempDevice **devices;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        OneWireTempBus(const JsonVariant &jsonObj);
        ~OneWireTempBus();
        
        OneWireTempDevice* GetFirstDevice();
        /** this function will search the devices to find the device with the uid */
        Device* findDevice(UIDPath& path) override;
        void requestTemperatures();
        void readAll();
        bool read(const HALReadStringRequestValue& val) override;

        String ToString();
    };

    class OneWireTempBusAtRoot : public OneWireTempBus {
    private:
        OneWireTempAutoRefresh autoRefresh;

    public:
        
        static Device* Create(const JsonVariant &jsonObj);

        OneWireTempBusAtRoot(const JsonVariant &jsonObj);
        ~OneWireTempBusAtRoot();

        void loop() override;
        // TODO implement the following methods to allow fine precision of paths
       // Device* findDevice(UIDPath& path) override;

    };
}