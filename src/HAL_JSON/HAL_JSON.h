#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_Manager.h"
#include "HAL_JSON_REST.h"

#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Parser.h"
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Script.h"

namespace HAL_JSON {
    void begin();
    void loop();
    
}