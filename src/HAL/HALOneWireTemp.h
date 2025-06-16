
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include "HALDevice.h"

namespace HAL {

    class OneWireTempDevice : public Device {
    public:
        float value;
        uint8_t romid[8];
        
        OneWireTempDevice(uint8_t romid[8]);
        ~OneWireTempDevice();
        
        bool read(HALValue &val);
        bool write(const HALValue &val);
        String ToString();
    };

    // this class is automatically instantiated even if there is only one 1 wire temperature sensor
    class OneWireBus {
    private:
        uint8_t pin;
        OneWireTempDevice *devices;
        uint32_t deviceCount = 0;
    public:
        OneWireBus();
        ~OneWireBus();
        
        /** this function will search the devices to find the device with the uid */
        Device* findDevice(uint64_t uid);
        void requestTemperatures();
        void readAll();
        bool read(HALValue &val);
        bool write(const HALValue &val);
        String ToString();
    };

    // this class is automatically instantiated even if there is only one 1 wire temperature sensor
    class OneWireTempGroup : public Device {
        enum State { IDLE, WAITING_FOR_CONVERSION };
    private:
        OneWireBus *busses;
        uint32_t busCount = 0;
        uint32_t refreshTimeMs = 0;
        uint32_t lastUpdateMs = 0;

        State state = IDLE;
        uint32_t lastStart = 0;
    public:
        OneWireTempGroup(JsonVariant jsonObj);
        ~OneWireTempGroup();
        
        /** this function will search the busses and their devices to find the device with the uid */
        Device* findDevice(uint64_t uid) override;
        void loop() override; // this task will take care of starting 1 wire temp measurements at regular intervalls (defined by refreshTimeMs)
        
        bool read(HALValue &val) override;
        bool write(const HALValue &val);
        String ToString() override;
    };
    
}