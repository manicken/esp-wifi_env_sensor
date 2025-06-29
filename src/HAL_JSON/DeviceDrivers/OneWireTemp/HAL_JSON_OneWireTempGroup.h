#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_DeviceTypeDefNames.h"
#include "../../ArduinoJSON_ext.h"
#include "HAL_JSON_OneWireTempBus.h"
#include "HAL_JSON_OneWireTempAutoRefresh.h"

namespace HAL_JSON {

    class OneWireTempGroup : public Device {

    private:
        OneWireTempAutoRefresh autoRefresh;
        OneWireTempBus **busses;
        uint32_t busCount = 0;

        void requestTemperatures();
        void readAll();

    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        
        OneWireTempGroup(const JsonVariant &jsonObj);
        ~OneWireTempGroup();
        
        /** this function will search the busses and their devices to find the device with the uid */
        Device* findDevice(UIDPath& path) override;

        void loop() override;
        bool read(const HALReadStringRequestValue &val) override;

    };
}