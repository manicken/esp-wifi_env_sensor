
#pragma once
#include <Arduino.h>
#include "../HAL_JSON_Device.h"
#include "HAL_JSON_RULE_Engine_Support.h"

namespace HAL_JSON {
    namespace Rules {

        /** StatementBlock contains either a IfBlock or a ExecBlock */
        struct StatementBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(StatementBlock);

            void* context;
            HALOperationResult (*handler)(void* context);
            Deleter deleter;

            StatementBlock();
            ~StatementBlock();
        };

        

    }
}