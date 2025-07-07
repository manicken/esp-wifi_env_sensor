
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"


namespace HAL_JSON {

    class TX433unit : public Device {
    private:
        
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        TX433unit(const JsonVariant &jsonObj, const char* type);
        Device* findDevice(UIDPath& path);
        bool write(const HALValue &val);
        bool write(const HALWriteStringRequestValue &val);

        String ToString() override;
    };
}