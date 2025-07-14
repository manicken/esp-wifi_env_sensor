#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_GPIO_Manager_REST.h"
#include "HAL_JSON_Manager.h"
#include "HAL_JSON_REST.h"

namespace HAL_JSON {
    void begin(WEBSERVER_TYPE &srv);
    void loop();
}