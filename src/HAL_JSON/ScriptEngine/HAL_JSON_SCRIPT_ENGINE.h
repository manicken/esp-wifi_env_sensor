/*

  <one line to give the program's name and a brief idea of what it does.>

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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