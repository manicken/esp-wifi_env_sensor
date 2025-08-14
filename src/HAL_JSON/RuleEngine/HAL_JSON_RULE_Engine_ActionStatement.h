
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"
#include "../HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {
    namespace Rules {

        struct ActionStatement
        {
            HAL_JSON_NOCOPY_NOMOVE(ActionStatement);

            CachedDeviceAccess* target;
            CalcRPNToken* sourceCalcItems;
            int sourceCalcItemsCount;

            ActionStatement();
            ~ActionStatement();
        };

    }
}