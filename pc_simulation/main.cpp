
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Parser.h"
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Expression_Parser.h"
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"
    #include "../src/HAL_JSON/HAL_JSON_Manager.h"
    #include "stubs/HAL_JSON_REST/HAL_JSON_REST.h"
    #include "../src/Support/ConvertHelper.h"
    #include "../src/Support/CharArrayHelpers.h"
    #include "../src/Support/ZeroCopyString.h"
    #include "RPN_tools.h"
    #include <ArduinoJson.h>
    #include "commandLoop.h"

    std::string halJsonRestCallback(const std::string& path) {
        HAL_JSON::ZeroCopyString zcCmd(path.c_str()+1); // +1 = remove leading /
        std::string message;
        HAL_JSON::CommandExecutor::execute(zcCmd, message);
        return message;
    }

    int main(int argc, char* argv[]) {
        std::cout << "********************************************************************" << std::endl;
        std::cout << "* WALHALLA rule development simulator - Running on Windows (MinGW) *" << std::endl;
        std::cout << "********************************************************************" << std::endl;

        std::string filename;
        std::string firstArg;
        std::string secondArg;

        // Check if a filename was given as a command-line argument
        if (argc > 1) {
            firstArg = argv[1];
            parseCommand(firstArg);
        }
        
        std::cout << "\n****** Starting REST api server:\n";
        HAL_JSON::REST::setup(halJsonRestCallback); // this will start the server
        std::cout << "\n****** Init HAL_JSON Manager\n";
        HAL_JSON::Manager::setup();
        std::thread cmdThread(commandLoop); // start command input thread from commandLoop that is in commandLoop.h
        while (running) { // running is in commandLoop.h
            HAL_JSON::Manager::loop();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        cmdThread.join(); // wait for command thread to finish
        std::cout << "Exited cleanly.\n";
        return 0;
    }