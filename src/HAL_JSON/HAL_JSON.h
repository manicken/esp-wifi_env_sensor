#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_Manager.h"
#include "HAL_JSON_REST.h"

#include "RuleEngine/HAL_JSON_RULE_Parser.h"
#include "RuleEngine/HAL_JSON_RULE_Engine.h"

namespace HAL_JSON {
    void begin();
    void loop();
}