
#pragma once


#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_TX433unit.h"

#define HAL_JSON_KEYNAME_TX433_UNITS "units"

namespace HAL_JSON {

    class TX433 : public Device {
    private:
        uint8_t pin = 0; // if pin would be used
        TX433unit** units;
        int unitCount;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        TX433(const JsonVariant &jsonObj, const char* type);
        TX433(TX433&) = delete;
        ~TX433();
        Device* findDevice(UIDPath& path);
        bool write(const HALWriteStringRequestValue &val);

        String ToString() override;
    };
}