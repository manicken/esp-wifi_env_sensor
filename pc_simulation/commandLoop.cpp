
#include "commandLoop.h"

std::atomic<bool> running{true};

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
        } else if (strncmp(cmd.c_str(), "hal/", 4) == 0) {

            HAL_JSON::ZeroCopyString subCmd(cmd.c_str() + 4);
            //std::cout << "\nhal cmd:" << subCmd.ToString() << "\n";
            
            
            std::string message;
            HAL_JSON::CommandExecutor::execute(subCmd, message);

            std::cout << message << "\n";
        } else {
            std::cout << "Unknown command: " << cmd << "\n";
        }
    }
}