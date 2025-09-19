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