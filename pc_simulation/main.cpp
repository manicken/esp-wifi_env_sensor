
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include <string_view>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Parser.h"
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Expression_Parser.h"

    #include "../src/HAL_JSON/HAL_JSON_Manager.h"
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine_Script.h"
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) // use this to avoid getting vscode error here
    #include "stubs/HAL_JSON_REST/HAL_JSON_REST.h"
#endif
    #include "../src/Support/ConvertHelper.h"
    #include "../src/Support/CharArrayHelpers.h"
    #include "../src/HAL_JSON/HAL_JSON_ZeroCopyString.h"
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
        int test = 5;
        int test2 = 0-1 * test;
        test2 *= 1;
        std::cout << "********************************************************************" << std::endl;
        std::cout << "* WALHALLA rule development simulator - Running on Windows (MinGW) *" << std::endl;
        std::cout << "********************************************************************" << std::endl;

        if (argc > 1) {
            // one shot tests
            parseCommand(argv[1], true); // true mean one short test
            HAL_JSON::Manager::CleanUp();
            return 0;
        }
        
        std::cout << "\n****** Starting REST api server:\n";
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) // use this to avoid getting vscode error here
        HAL_JSON::REST::setup(halJsonRestCallback); // this will start the server
#endif
        std::cout << "\n****** Init HAL_JSON Manager\n";
        HAL_JSON::Manager::setup();
        HAL_JSON::Rules::ScriptsBlock::ValidateAndLoadAllActiveScripts();
        std::cout << "\n****** Starting commandLoop thread\n";
        std::thread cmdThread(commandLoop); // start command input thread from commandLoop that is in commandLoop.h
        while (running) { // running is in commandLoop.h
            //HAL_JSON::Manager::loop();
            HAL_JSON::Rules::ScriptsBlock::Exec(); // runs the scriptengine
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        cmdThread.join(); // wait for command thread to finish
        std::cout << "Exited cleanly.\n" << std::flush;
        HAL_JSON::Manager::CleanUp();
        return 0;
    }