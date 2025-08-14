
#pragma once
#include <Arduino.h>
#include "../HAL_JSON_Device.h"
#include "HAL_JSON_RULE_Engine_Support.h"

namespace HAL_JSON {
    namespace Rule {

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

        /** collection of StatementBlock(s) */
        struct StatementBlocks
        {
            HAL_JSON_NOCOPY_NOMOVE(StatementBlocks);

            StatementBlock* items;
            int itemsCount;

            /** used to execute all opItems one after annother */
            HALOperationResult (*Exec)(void);

            /** returns true if exec should run */
            bool (*ShouldExec)(void);

            StatementBlocks();
            ~StatementBlocks();
        };

    }
}