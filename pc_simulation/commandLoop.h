#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/Support/ZeroCopyString.h"
#include "../src/HAL_JSON/HAL_JSON_CommandExecutor.h"

extern std::atomic<bool> running;

void commandLoop();
