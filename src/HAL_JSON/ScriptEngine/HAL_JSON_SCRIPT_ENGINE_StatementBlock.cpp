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

#include "HAL_JSON_SCRIPT_ENGINE_StatementBlock.h"
#include "HAL_JSON_SCRIPT_ENGINE_IfStatement.h"
#include "../HAL_JSON_Device.h"
#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_IfStatement.h"
#include "HAL_JSON_SCRIPT_ENGINE_ActionStatement.h"

namespace HAL_JSON {
    namespace ScriptEngine {

        StatementBlock::StatementBlock () : context(nullptr), handler(nullptr), deleter(nullptr) { }

        StatementBlock::~StatementBlock ()
        {
            if (deleter) { deleter(context); context=nullptr; }
        }

        HALOperationResult dummyActionHandler(void* context) {
            return HALOperationResult::Success;
        } 

        void StatementBlock::Set(Tokens& tokens)
        {
            //printf("(%d) StatementBlock::Set----------------- : %s\n",tokens.currIndex, tokens.Current().ToString().c_str());
            // note to myself, this is one statement consume only

            Token& token = tokens.Current();
 
            if (token.type == TokenType::If) {
                //printf("---- StatementBlock::Set was if\n");
                IfStatement* newItem = new IfStatement(tokens);
                context = newItem;
                handler = IfStatement::Handler;
                deleter = DeleteAs<IfStatement>;
                
            } else if (token.type == TokenType::Action) {
                tokens.currentEndIndex = tokens.currIndex + token.itemsInBlock;
                //printf("--------------------- StatementBlock::Set was action: %s\n",tokens.SliceToString().c_str());
                tokens.currentEndIndex = tokens.count;
                //tokens.currIndex += token.itemsInBlock; // just skip it here
                ActionStatement* newItem = new ActionStatement(tokens, handler);
                context = newItem;
                
                deleter = DeleteAs<ActionStatement>;
                //handler = dummyActionHandler;
            }
            else {
                ReportTokenError(token, "StatementBlock::Set !!!! very big issue found unknown type:", TokenTypeToString(token.type));
            }
            
        }

    }
}