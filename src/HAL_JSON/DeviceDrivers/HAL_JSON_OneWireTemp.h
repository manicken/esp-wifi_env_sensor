
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../../Support/Logger.h"
#include "../../Support/ConvertHelper.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_DeviceTypeDefNames.h"
#include "../ArduinoJSON_ext.h"


namespace HAL_JSON {

    #define HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS 1000

    namespace OneWireTemp {
        bool VerifyJSON(const JsonVariant &jsonObj);
        Device* Create(const JsonVariant &jsonObj);
        double ParseRefreshTime(const JsonVariant &jsonObj);
        uint32_t ParseRefreshTimeMs(const JsonVariant &value);
        enum class Type {
            GROUP,
            BUS,
            DEVICE
        };
        enum class TempFormat {
            Celsius,
            Fahrenheit
        };
    }

    class OneWireTempDevice : public Device {
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        OneWireTemp::TempFormat format = OneWireTemp::TempFormat::Celsius;
        float value;
        uint8_t romid[8];
        
        OneWireTempDevice(const JsonVariant &jsonObj);
        ~OneWireTempDevice();
        
        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest&req) override;
        String ToString();
    };

    // this class is automatically instantiated even if there is only one 1 wire temperature sensor
    class OneWireTempBus {
    private:
        uint8_t pin;
       
        uint32_t deviceCount = 0;
        OneWireTempDevice **devices;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        OneWireTempBus(const JsonVariant &jsonObj, OneWireTemp::Type type);
        ~OneWireTempBus();
        
        OneWireTempDevice* GetFirstDevice();
        /** this function will search the devices to find the device with the uid */
        Device* findDevice(uint64_t uid);
        void requestTemperatures();
        void readAll();
        //bool read(const HALReadRequest &req); currently Bus do not use uid so this func do not make any sense
        //bool write(const HALWriteRequest &req); currently Bus do not use uid so this func do not make any sense
        String ToString();
    };

    // this class is automatically instantiated even if there is only one 1 wire temperature sensor, to avoid creating same Refresh Loop state machine for every other type
    class OneWireTempGroup : public Device {
        enum class State { IDLE, WAITING_FOR_CONVERSION };
    private:
        OneWireTemp::Type type = OneWireTemp::Type::GROUP; 
        OneWireTempBus **busses;
        uint32_t busCount = 0;
        uint32_t refreshTimeMs = HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS;
        uint32_t lastUpdateMs = 0;

        State state = State::IDLE;
        uint32_t lastStart = 0;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        OneWireTempGroup(const JsonVariant &jsonObj);
        ~OneWireTempGroup();
        
        /** this function will search the busses and their devices to find the device with the uid */
        Device* findDevice(uint64_t uid) override;
        void loop() override; // this task will take care of starting 1 wire temp measurements at regular intervalls (defined by refreshTimeMs)
        
        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest&req) override;
        String ToString() override;
    };
    
}