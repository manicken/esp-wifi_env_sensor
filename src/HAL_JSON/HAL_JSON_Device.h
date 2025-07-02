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

    enum class UIDPathMaxLength : uint8_t {
        One = 1,
        Two = 2,
        Three = 3,
        Four = 4 // not used at the moment
    };

    class Device {
    protected:
        Device() = delete;
        const char* type;
    public:
        Device(UIDPathMaxLength uidMaxLength, const char* type);
        virtual ~Device();

        uint64_t uid;
        const uint8_t uidMaxLength;

        virtual bool read(HALValue &val);
        virtual bool write(const HALValue &val);
        virtual bool read(const HALReadStringRequestValue &val);
        virtual bool write(const HALWriteStringRequestValue &val);
        virtual bool read(const HALReadValueByCmd &val);
        virtual void loop();
        /** used to find sub/leaf devices @ "group devices" */
        virtual Device* findDevice(UIDPath& path);

        virtual String ToString();

        static bool DisabledInJson(const JsonVariant& jsonObj);
    };

#define HAL_JSON_DEVICE_CONST_STRINGS_USE_F_PREFIX

#ifdef HAL_JSON_DEVICE_CONST_STRINGS_USE_F_PREFIX
#define HAL_JSON_DEVICE_CONST_STR_DECLARE(name) extern const __FlashStringHelper* name
#define HAL_JSON_DEVICE_CONST_STR_DEFINE(name, value) const __FlashStringHelper* name = F(value)
#else
#define HAL_JSON_DEVICE_CONST_STR_DECLARE(name) extern const char* name
#define HAL_JSON_DEVICE_CONST_STR_DEFINE(name, value) const char* name = value
#endif

    namespace DeviceConstStrings {
        HAL_JSON_DEVICE_CONST_STR_DECLARE(uid);
        /** "\"type\":\"" */
        HAL_JSON_DEVICE_CONST_STR_DECLARE(type);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(pin);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(value);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(valueStartWithComma);
        HAL_JSON_DEVICE_CONST_STR_DECLARE(refreshTimeMs);
    }
} // namespace HAL
