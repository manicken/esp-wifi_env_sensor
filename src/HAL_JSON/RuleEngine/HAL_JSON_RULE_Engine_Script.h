
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_StatementBlock.h"
#include "HAL_JSON_RULE_Parser.h"
#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {
        /**
         * A single trigger definition at the root level of a script.
         * Each trigger is linked to one or more executable operation blocks.
         */
        struct TriggerBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(TriggerBlock);

            /** the function set to this pointer should return true if the StatementBlock(s) should execute */
            //bool (*triggerSource)();
            bool (*triggerSource)(void* context);
            void* context;  // optional — lets the triggerSource read its own state
            StatementBlock* items;
            int itemsCount;

            static bool AllwaysRun(void* context);

            TriggerBlock();
            ~TriggerBlock();

            HALOperationResult Exec();

            void Set(int statementBlockCount, Tokens& tokens);
        };

        /**
         * Represents a single script file.
         * A script may contain one or more top-level triggers.
         * A script may contain IfBlocks at the top-level
         * in such cases the TriggerBlock::AllwaysRun should be used
         * as the trigger source
         */
        struct ScriptBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(ScriptBlock);

            TriggerBlock* triggerBlocks;
            int triggerBlockCount;

            ScriptBlock();
            ~ScriptBlock();

            void Set(Tokens& tokens);

            void Exec();
        };

        /**
         * Global container for all loaded scripts in the engine.
         * This is the highest-level structure in the script engine hierarchy.
         */
        struct ScriptsBlock
        {
            static ScriptBlock* scriptBlocks;
            static int scriptBlocksCount;
            static int currentScriptIndex;

            /** just a callback wrapper to begin initializing the structures */
            static void ScriptFileParsed(Tokens& tokens);
            /** should be run before using LoadAllActiveScripts */
            static bool ValidateAllActiveScripts();
            /** ValidateAllActiveScripts should be run before using this function */
            static bool LoadAllActiveScripts();
            /** begins with validating all scripts
             * and if all pass then it begins to load in the structures
             */
            static bool ValidateAndLoadAllActiveScripts();
            /** entry point of one script loop iteraction */
            static void Exec();
        };
    }
}