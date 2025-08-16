
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_Device.h"
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Parser_Token.h"
#include "HAL_JSON_RULE_Engine_ActionStatement.h"


namespace HAL_JSON {
    namespace Rules {

        // Forward declare IfBlock, currently not needed but keep it here if we somewhere in the future want to do it
        struct IfBlock;

        /** StatementBlock contains either a IfBlock or a ExecBlock */
        struct StatementBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(StatementBlock);

            void* context;
            HALOperationResult (*handler)(void* context);
            Deleter deleter;

            StatementBlock();
            ~StatementBlock();
            
            void Set(Tokens& tokens);
        };

        

    }
}