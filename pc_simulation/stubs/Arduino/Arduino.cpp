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

#include "Arduino.h"
#include <iostream>

SerialStub Serial;
//uint64_t lastmicros = 0;
void digitalWrite(int pin, int val) {
    //using namespace std::chrono;
    //auto now = high_resolution_clock::now();
    //uint64_t micros = duration_cast<microseconds>(now.time_since_epoch()).count();

    //std::cout << "[" << micros-lastmicros << " uS] digitalWrite(" << pin << ", " << val << ")\n";
    std::cout << "digitalWrite(" << pin << ", " << val << ")\n";
    //lastmicros = micros;
}