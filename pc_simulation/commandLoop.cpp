
#include "commandLoop.h"

std::atomic<bool> running{true};
void ParseHelpCommand(HAL_JSON::ZeroCopyString& zcCmd) {
    HAL_JSON::ZeroCopyString zcCmdHelp = zcCmd.SplitOffHead('/');
    if (zcCmdHelp == "hal") {
        HAL_JSON::ZeroCopyString zcCmdHelpHal = zcCmd.SplitOffHead('/');
        if (zcCmdHelpHal == "read") {
            std::cout << "Format: hal/read/<uint32/bool/float/string>/<deviceUID>/<optional cmd>\n";
        } else if (zcCmdHelpHal == "write") {
            std::cout << "Format: hal/write/<uint32/bool/float/string>/<deviceUID>/<value>\n";
        } else if (zcCmdHelpHal == "reloadcfg") {
            std::cout << "Format: hal/reloadcfg/<optional filename>\n";
        } else {
            std::cout << "Available commands: read, write, reloadcfg\n";
        }
    } else if (zcCmdHelp == "exit") {
        std::cout << "exits this program\n";
    } else if (zcCmdHelp == "status") {
        std::cout << "show program status\n";
    } else if (zcCmdHelp == "help") {
        std::cout << "shows this help\n";
    } else if (zcCmdHelp.Length() != 0) {
        std::cout << "Unknown help chapter: " << zcCmdHelp.ToString() << "\n";
        std::cout << "Available chapters: exit, status, help, hal\n";
    } else {
        std::cout << "Available commands: exit, status, help, hal\n";
    }
}
void exprTestLoad(HAL_JSON::ZeroCopyString& zcStr) {
    HAL_JSON::ZeroCopyString zcFilePath = zcStr.SplitOffHead('/');
    std::string strFilePath = zcFilePath.ToString();
    size_t fileSize = 0;
    char* contents;// = HAL_JSON::ReadFileToMutableBuffer(filename.c_str(), fileSize);
    
    if (LittleFS_ext::load_from_file(strFilePath.c_str(), &contents, &fileSize) == false) {
        std::cout << "Error: file empty or could not be found: " << strFilePath << "\n";
    } 
    HAL_JSON::Rules::Expressions::ValidateExpression(contents);
    delete[] contents;
}
void parseCommand(std::string cmd) {
    HAL_JSON::ZeroCopyString zcCmd(cmd.c_str());

    HAL_JSON::ZeroCopyString zcCmdRoot = zcCmd.SplitOffHead('/');

    if (zcCmdRoot == "exit" || zcCmdRoot == "e") {
        std::cout << "Shutting down...\n";
        running = false;
    } else if (zcCmdRoot == "status") {
        std::cout << "Status: running\n";
    } else if (zcCmdRoot == "help") {
        ParseHelpCommand(zcCmd);            
    } else if (zcCmdRoot == "hal") {
        std::string message;
        HAL_JSON::CommandExecutor::execute(zcCmd, message);
        std::cout << message << "\n";
    } else if (zcCmdRoot == "texpr") {
        exprTestLoad(zcCmd);
    } else if (zcCmdRoot == "loadruleset") {
        HAL_JSON::ZeroCopyString zcFilePath = zcCmd.SplitOffHead('/');
        std::string filePath;
        if (zcFilePath.Length() == 0)
            filePath = zcFilePath.ToString();
        else
            filePath = "ruleset.txt";
        HAL_JSON::Rules::Parser::ReadAndParseRuleSetFile(filePath.c_str());
    } else {
        std::cout << "Unknown command: " << cmd << "\n";
    }
}
void commandLoop() {
    std::string cmd;
    while (running) {
        std::cout << "> ";
        if (!std::getline(std::cin, cmd)) break;
        parseCommand(cmd);
    }
}