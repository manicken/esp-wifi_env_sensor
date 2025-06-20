#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <Arduino.h> // Needed for String class
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "GPIO_manager.h"

namespace HAL_JSON {

    class Device {
    protected:
        Device();
    public:
        virtual ~Device();

        uint64_t uid;

        virtual bool read(const HALReadRequest &req);
        virtual bool write(const HALWriteRequest &val);
        virtual bool read(const HALReadStringRequest &val);
        virtual bool write(const HALWriteStringRequest &val);
        virtual void loop();
        /** used to find sub/leaf devices @ "group devices" */
        virtual Device* findDevice(uint64_t uid);

        virtual String ToString();
    };
} // namespace HAL
