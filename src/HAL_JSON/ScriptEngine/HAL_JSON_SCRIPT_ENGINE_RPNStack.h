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
#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        template<typename T>
        class RPNStack {
        public:
            HAL_JSON_NOCOPY_NOMOVE(RPNStack);

            int size;
            T* items;
            int sp;

            RPNStack() : size(0), items(nullptr), sp(0) {}
            explicit RPNStack(int size) { Init(size); }

            ~RPNStack() { delete[] items; }

            void Init(int newSize) {
                delete[] items; // free old memory
                size = newSize;
                sp = 0;
                items = new T[size];
            }

            bool GetFinalResult(T& value) {
                if (sp != 1) return false;
                value = items[0];
                return true;
            }

        };
        
        
    }
}