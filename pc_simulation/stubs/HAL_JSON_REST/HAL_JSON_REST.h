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
#ifdef _WIN32
#ifdef INPUT
#undef INPUT
#endif
#include <httplib.h>  // Add to your project
#endif
#include <functional>
#include <string>

#define HAL_JSON_REST_API_PORT 82

using HAL_JSON_REST_CB = std::function<std::string(const std::string&)>;

namespace HAL_JSON {
    class REST {
    private:
#ifdef _WIN32
        static httplib::Server server;
#endif
        static HAL_JSON_REST_CB callback;
    public:
        static void setup(HAL_JSON_REST_CB cb);
    };
}