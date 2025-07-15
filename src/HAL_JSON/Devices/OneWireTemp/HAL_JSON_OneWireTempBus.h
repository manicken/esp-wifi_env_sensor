#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "HAL_JSON_OneWireTempDevice.h"
#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    class OneWireTempBus : public Device {
    private:
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
        bool haveDeviceWithRomID(OneWireAddress addr);
    public:
        int deviceCount = 0;
        OneWireTempDevice **devices;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        OneWireTempBus(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempBus();
        
        OneWireTempDevice* GetFirstDevice();
        /** this function will search the devices to find the device with the uid */
        Device* findDevice(UIDPath& path) override;
        void requestTemperatures();
        void readAll();
        bool read(const HALReadStringRequestValue& val) override;

        std::string getAllDevices(bool printTemp = false, bool onlyNewDevices = false);
        String ToString() override;
    };

    class OneWireTempBusAtRoot : public OneWireTempBus {
    private:
        OneWireTempAutoRefresh autoRefresh;

    public:
        
        static Device* Create(const JsonVariant &jsonObj, const char* type);

        OneWireTempBusAtRoot(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempBusAtRoot();

        void loop() override;
        
        String ToString() override;
    };
}