#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <Arduino.h> // Needed for String class
#include "HALValue.h"
#include "UIDPath.h"

namespace HAL {

    class Device {
    protected:
        Device();
    public:
        virtual ~Device();

        uint64_t uid;

        virtual bool read(HALValue &val) = 0;
        virtual bool write(const HALValue &val) = 0;
        virtual bool read(String &val);
        virtual bool write(String val);
        virtual void loop();
        /** used to find sub/leaf devices @ "group devices" */
        virtual Device* findDevice(uint64_t uid);

        virtual String ToString();
    };
} // namespace HAL
