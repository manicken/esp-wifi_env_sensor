
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_Device.h" // HALOperationResult
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"
#include "../HAL_JSON_CachedDeviceAccess.h"
#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {

        struct ActionStatement
        {
            HAL_JSON_NOCOPY_NOMOVE(ActionStatement);

            CachedDeviceAccess* target;
            CalcRPNToken* sourceCalcItems;
            int sourceCalcItemsCount;

            ActionStatement(Tokens& tokens);
            ~ActionStatement();

            static HALOperationResult Handler(void* context);
        };

    }
}