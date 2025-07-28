#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic<bool> running = true;

void commandLoop() {
    std::string cmd;
    while (running) {
        std::cout << "> ";
        if (!std::getline(std::cin, cmd)) break;

        if (cmd == "exit") {
            std::cout << "Shutting down...\n";
            running = false;
        } else if (cmd == "status") {
            std::cout << "Status: running\n";
        } else if (cmd == "help") {
            std::cout << "Available commands: exit, status, help\n";
        } else {
            std::cout << "Unknown command: " << cmd << "\n";
        }
    }
}
