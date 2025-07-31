#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/Support/ZeroCopyString.h"
#include "../src/HAL_JSON/HAL_JSON_CommandExecutor.h"
#include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Parser.h"
#include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Expression_Parser.h"

extern std::atomic<bool> running;

void commandLoop();
void parseCommand(std::string cmd);
