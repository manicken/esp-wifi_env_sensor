
#pragma once

#include <Arduino.h>
//#include "../HAL_JSON_Device.h"
#include "HAL_JSON_SCRIPT_ENGINE_Support.h" // Deleter
//#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_IfStatement.h"
//#include "HAL_JSON_SCRIPT_ENGINE_ActionStatement.h"


namespace HAL_JSON {
    namespace ScriptEngine {

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