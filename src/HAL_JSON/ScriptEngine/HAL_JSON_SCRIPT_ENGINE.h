
#pragma once

#include <Arduino.h>

#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Script.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        /**
         * Global container for all loaded scripts in the engine.
         * This is the highest-level structure in the script engine hierarchy.
         */
        struct ScriptsBlock
        {
            /** set this to true to make the script run,
             * set to false to make the script not run 
             * this is set to false if any of the script validate
             * fails
             * */
            static bool running;
            static ScriptBlock* scriptBlocks;
            static int scriptBlocksCount;
            static int currentScriptIndex;

            /** just a callback wrapper to begin initializing the structures */
            static void ScriptFileParsed(Tokens& tokens);
            
            /** ValidateAllActiveScripts should be run before using this function */
            static bool LoadAllActiveScripts();
            
        };
        /** should be run before using LoadAllActiveScripts */
        bool ValidateAllActiveScripts();
        /** begins with validating all scripts
         * and if all pass then it begins to load in the structures
         */
        bool ValidateAndLoadAllActiveScripts();
        /** entry point of one script loop iteraction */
        void Exec();
    }
}