
#pragma once

#ifndef _WIN32
#include <Arduino.h>
#include <ArduinoJson.h>
#endif
#include <stdlib.h>

#include "../Support/Logger.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Device.h"
#include "HAL_JSON_Manager.h"

namespace HAL_JSON {

    class CachedDevicePath {
    private:
        int cachedVersion;
        UIDPath path;
        Device* device;
    public:
        CachedDevicePath(const char* uidPath);
        Device* GetDevice();
    };

}