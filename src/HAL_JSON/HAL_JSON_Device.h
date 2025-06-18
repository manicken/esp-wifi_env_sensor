#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <Arduino.h> // Needed for String class
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "UID_JSON_Path.h"
#include "GPIO_manager.h"

namespace HAL_JSON {

    class Device {
    protected:
        Device();
    public:
        virtual ~Device();

        uint64_t uid;

        virtual bool read(HALValue &val);
        virtual bool write(const HALValue &val);
        virtual bool read(String &val);
        virtual bool write(const String &val);
        virtual void loop();
        /** used to find sub/leaf devices @ "group devices" */
        virtual Device* findDevice(uint64_t uid);

        virtual String ToString();
    };
} // namespace HAL
