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
#include "HAL_JSON_OneWireTempAutoRefreshDevice.h"

namespace HAL_JSON {

    class OneWireTempBusBase {
    protected:
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
       
        uint32_t deviceCount = 0;
        OneWireTempDevice **devices;
        
    };

    class OneWireTempBus : public Device, public OneWireTempBusBase {
        
    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        OneWireTempBus(const JsonVariant &jsonObj);
        ~OneWireTempBus();
        
        OneWireTempDevice* GetFirstDevice();
        /** this function will search the devices to find the device with the uid */
        Device* findDevice(const UIDPath& path) override;
        void requestTemperatures();
        void readAll();
        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest &req) override;
        String ToString();
    };

    class OneWireTempBusAtRoot : public OneWireTempAutoRefreshDevice, public OneWireTempBusBase {
    private:
        void requestTemperatures() override;
        void readAll() override;

    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);

        OneWireTempBusAtRoot(const JsonVariant &jsonObj);
        ~OneWireTempBusAtRoot();
        
        OneWireTempDevice* GetFirstDevice();
        /** this function will search the devices to find the device with the uid */
        Device* findDevice(const UIDPath& path) override;

        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest &req) override;
        String ToString() override;
    };
}