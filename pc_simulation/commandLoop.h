#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/HAL_JSON/HAL_JSON_ZeroCopyString.h"
#include "../src/HAL_JSON/HAL_JSON_CommandExecutor.h"
#include "../src/HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Parser.h"
#include "../src/HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"
#include "../src/HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "../src/HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"

extern std::atomic<bool> running;

void commandLoop();
void parseCommand(const char* cmd, bool oneShot = false);
