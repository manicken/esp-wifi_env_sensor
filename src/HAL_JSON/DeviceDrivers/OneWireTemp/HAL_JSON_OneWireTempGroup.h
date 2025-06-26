#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_DeviceTypeDefNames.h"
#include "../../ArduinoJSON_ext.h"
#include "HAL_JSON_OneWireTempBus.h"
#include "HAL_JSON_OneWireTempAutoRefreshDevice.h"

namespace HAL_JSON {

    class OneWireTempGroup : public OneWireTempAutoRefreshDevice {

    private:
        OneWireTempBus **busses;
        uint32_t busCount = 0;

        void requestTemperatures() override;
        void readAll() override;

        
        
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        
        OneWireTempGroup(const JsonVariant &jsonObj);
        ~OneWireTempGroup();
        
        /** this function will search the busses and their devices to find the device with the uid */
        Device* findDevice(const UIDPath& path) override;

        bool read(const HALReadRequest &req) override;
        bool write(const HALWriteRequest&req) override;
        String ToString() override;
    };
}