
    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Parser.h"
    #include "../src/HAL_JSON/RuleEngine/HAL_JSON_RULE_Engine.h"
    #include "../src/HAL_JSON/HAL_JSON_Manager.h"
    #include "stubs/HAL_JSON_REST/HAL_JSON_REST.h"
    #include "../src/Support/ConvertHelper.h"
    #include "../src/Support/CharArrayHelpers.h"
    #include "../src/Support/ZeroCopyString.h"
    #include "RPN_tools.h"


    #include <ArduinoJson.h>

    #include "commandLoop.h"
    
namespace  HAL_JSON
{
    
    void ReportError(const char* msg) {
        std::cout << "Error: " << msg << std::endl;
    }

    char* ReadFileToMutableBuffer(const std::string& filename, size_t& outSize) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) return nullptr;

        outSize = file.tellg();
        if (outSize == 0) return nullptr;
        file.seekg(0);

        // Allocate mutable buffer (+1 for null terminator if needed)
        char* buffer = new char[outSize + 1];
        file.read(buffer, outSize);
        buffer[outSize] = '\0'; // optional if you need null-terminated text

        return buffer;
    }

    bool VerifyConditionBlocks(Token* tokens, int tokenCount) {
        bool anyError = false;
        for (int i = 0; i < tokenCount; ++i) {
            Token& token = tokens[i];
            if ((IsType(token, "if") || IsType(token, "elseif")) == false) continue;
            const char* conditions = tokens[i+1].text;
            std::cout << conditions << std::endl;
            // parantesis could be allowed in a later version
            // do checks here
            // first 'split' by ||
            // and then by &&
            // or as this is only a check, then I could 'split' by either && or ||
            // loop here and verify each condition
            // verify that they only contain one relational operator each
            // verify the left and right operands
            // that they are valid device names and valid const values

        }
        return anyError == false;
    }

    

    bool CountOperatorsAndOperands(const char* expr, int& operatorCount, int& operandCount, int& leftParenthesisCount ) {
        operatorCount = 0;
        operandCount = 0;
        leftParenthesisCount  = 0;
        // early checks
        if (expr == nullptr) {
            ReportError("expr. is nullptr");
            return false;
        }
        if (strlen(expr) == 0) {
            ReportError("expr. is empty");
            return false;
        }
        if(IsOperator(*expr)) {
            ReportError("expr. cannot start with a operator");
            return false;
        }

        int rightParenthesisCount = 0;

        bool inOperand = false;
        
        for (const char* p = expr; *p != '\0'; ++p) {
            if (IsOperator(*p)) {
                operatorCount++;
                inOperand = false;
            } else if (*p == '(') {
                leftParenthesisCount++;
                inOperand = false;
            } else if (*p == ')') {
                rightParenthesisCount++;
                inOperand = false;
            }
            else if (!inOperand) {
                operandCount++;
                inOperand = true;
            }
        }
        bool anyError = false;
        if (operatorCount >= operandCount) {
            ReportError("double operator(s) detected");
            anyError = true;
        } else if (operatorCount != operandCount - 1) {
            ReportError("operator(s) missing before/after parenthesis");
            anyError = true;
        }
        if (leftParenthesisCount != rightParenthesisCount) {
            ReportError("mismatch parenthesis detected");
            anyError = true;
        }

        return anyError == false;
    }
    
    void GetOperands(const char* str, ZeroCopyString* operands, int operandCount) {

        bool inOperand = false;
        int operandIndex = 0;
        const char* p = str;
        for ( ; *p != '\0' ; ++p) {
            if (IsOperator(*p) || *p == '(' || *p == ')') {
                if (inOperand) {
                    if (operandIndex == operandCount) { // out of bounds should never happend
                        ReportError("something is very wrong");
                    } else
                        operands[operandIndex++].end = p;
                    
                }
                inOperand = false;
            }
            else if (!inOperand) {
                operands[operandIndex].start = p;
                inOperand = true;
            }
        }
        // catch the last operand if any
        if (inOperand) {
            if (operandIndex == operandCount) { // out of bounds should never happend
                ReportError("something is very wrong");
            } else {
                operands[operandIndex].end = p;
            }
        }
    }

    bool OperandIsVariable(const ZeroCopyString& operand) {
        const char* p = operand.start;
        const char* const end = operand.end;
        while (p < end) {
            if (isdigit(*p) == false) return true;
            p++;
        }
        return false;
    }
    
    bool ValidateExpression(const char* str) {
        int operatorCount, operandCount, leftParenthesisCount ;
        bool anyError = false;

        anyError = (false == CountOperatorsAndOperands(str, operatorCount, operandCount, leftParenthesisCount));
        // allways print debug info
        std::cout << "\noperatorCount:" << operatorCount << ", operandCount:" << operandCount << ", leftParenthesisCount :" << leftParenthesisCount  << std::endl;
        if (anyError) return false;
        ZeroCopyString* operands = new ZeroCopyString[operandCount];

        GetOperands(str, operands, operandCount);
        // just debug info
        for (int i=0;i<operandCount;i++) {
            const ZeroCopyString& operand = operands[i];
            std::cout << "isVar:" << OperandIsVariable(operand) << ", operand: " + operand.ToString()  << "\n";
        }
        std::cout << "\n";
        for (int i=0;i<operandCount;i++) {
            const ZeroCopyString& operand = operands[i];

            if (OperandIsVariable(operand)) {
                ZeroCopyString varOperand = operand;
                // here we need to check if the variablename is a (logical/physical/virtual) device
                const char* funcNameSeparator = varOperand.FindChar('#');
                ZeroCopyString funcName;
                if (funcNameSeparator) {
                    funcName.start = funcNameSeparator+1;
                    funcName.end = varOperand.end;
                    varOperand.end = funcNameSeparator;
                }
                std::cout << "varOperand before: " << varOperand.ToString() << ", ";
                UIDPath path(varOperand);
                std::cout << "uidPath decoded: " << path.ToString();
                if (funcNameSeparator) {
                    std::cout << ", funcName: " << funcName.ToString();
                }
                std::cout << "\n";
                // now we can actually 
                // 1. check if the device exists
                // 2. also if a device exists in a expression that 
                //    also mean that that device MUST support read function
                //    so we also need to check if the device read function
                //    returns true
                // 3. if funcName is provided we can also verify that
                //
                // if either above fail then the rule that this expresssion normally 
                // belongs to should be rejected as invalid 
            }
        }


        delete[] operands;
        return true;
    }

    bool ParseRuleSet(Token* tokens, char* fileContents, int tokenCount) {
        if (Tokenize(fileContents, tokens, tokenCount) == false) {
            std::cout << "Error: could not Tokenize" << std::endl;
            return false;
        }
        
        for (int i=0;i<tokenCount;i++) {
            std::cout << "Token("<<i<<"): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ")\t" << tokens[i].text << std::endl;
        }

        std::cout << std::endl << "VerifyBlocks (BetterError): ";
        if (VerifyBlocks(tokens, tokenCount) == false) {
            std::cout << "[FAIL]" << std::endl;
            return false;
        }
        std::cout << "[OK]" << std::endl << std::endl;
        // if here then we can safely parse all blocks

        std::cout << std::endl << "MergeConditions: ";
        MergeConditions(tokens, tokenCount);
        std::cout << "[OK]" << std::endl;
        
        std::cout << std::endl << "MergeActions: ";
        MergeActions(tokens, tokenCount);
        std::cout << "[OK]" << std::endl;
        
        std::cout << std::endl << "CountBlockItems: ";
        CountBlockItems(tokens, tokenCount);
        std::cout << "[OK]" << std::endl;
        
        std::cout << std::endl << "EnsureActionBlocksContainItems: " << std::endl;
        if (EnsureActionBlocksContainItems(tokens, tokenCount) == false) {
            std::cout << "[FAIL]" << std::endl;
            return false;
        }
        std::cout << "[OK]" << std::endl;
            
        for (int i=0;i<tokenCount;i++) {
            std::cout << "Token(" << i << "): " << "(line:" << std::to_string(tokens[i].line) << ", col:" << std::to_string(tokens[i].column) << ", itemCount:" << tokens[i].itemsInBlock << ")\t" << tokens[i].text << std::endl;
        }

        std::cout << std::endl << "VerifyConditionBlocks: " << std::endl;
        if (VerifyConditionBlocks(tokens, tokenCount) == false) {
            std::cout << "[FAIL]" << std::endl;
            return false;
        }
        return true;
    }

    bool ReadAndParseRuleSetFile(const char* filePath) {
        size_t fileSize;
        char* fileContents = ReadFileToMutableBuffer(filePath, fileSize);
        if (fileContents == nullptr) {
            std::cout << "Error: file could not be read/or is empty" << std::endl;
            return false;
        }
        
        FixNewLines(fileContents);
        StripComments(fileContents); // count_tokens currently wont work without using this
        std::cout << std::endl << "FileContents (after comments removed and newlines fixed): " << std::endl;
        std::cout << fileContents << std::endl;

        int tokenCount = CountTokens(fileContents);
        std::cout << "Token count: " << tokenCount << std::endl;
        Token* tokens = new Token[tokenCount];
        bool anyError = false;
        if (ParseRuleSet(tokens, fileContents, tokenCount)) {
            std::cout << "ParseRuleSet [OK]" << std::endl;
        } else {
            std::cout << "ParseRuleSet [FAIL]" << std::endl;
            anyError = true;
        }
        // dont forget to free/delete
        delete[] fileContents;
        delete[] tokens;
        return anyError == false;
    }
}

/********************************************************************************************************** */

std::string myCallback(const std::string& path) {
    HAL_JSON::ZeroCopyString zcCmd(path.c_str()+1); // +1 = remove leading /
    std::string message;
    HAL_JSON::CommandExecutor::execute(zcCmd, message);
    return message;
}

    int main(int argc, char* argv[]) {
        std::cout << "WALHALLA rule development simulator - Running on Windows (MinGW)" << std::endl;

        

        std::string filename;
        std::string firstArg;
        std::string secondArg;

        size_t jsonFileSize = 0;
        char* jsonContents = HAL_JSON::ReadFileToMutableBuffer("cfg.json", jsonFileSize);
        size_t jsonDocBufferSize = (size_t)((float)jsonFileSize * 2.0f);
        DynamicJsonDocument jsonDoc(jsonDocBufferSize);
        DeserializationError error = deserializeJson(jsonDoc, jsonContents);
        if (error)
        {
            delete[] jsonContents;
            std::cout << "ReadJSON - deserialization failed: " << error.c_str() << "\n";
            //GlobalLogger.Error(F("ReadJSON - deserialization failed: "), error.c_str());
            //return false;
        } else {
            std::string memUsage = std::to_string(jsonDoc.memoryUsage()) + " of " + std::to_string(jsonDoc.capacity());
            std::cout << "jsonDoc.memoryUsage=" << memUsage.c_str() << "\n";
            std::cout << "json root item count:" << jsonDoc.size() << "\n";

            delete[] jsonContents;
        }
        
        // Check if a filename was given as a command-line argument
        if (argc > 1) {
            firstArg = argv[1];
            //std::cout << "first arg:" << firstArg << std::endl;
            if (argc > 2) {
                secondArg = argv[2];
                //std::cout << "second arg:" << secondArg << std::endl;
                if (strncmp(firstArg.c_str(), "--", 2) == 0) {
                    if (firstArg == "--rule") {
                        filename = secondArg;
                        
                    } else if (firstArg == "--expr") {
                        filename = secondArg;
                        std::cout << "Using provided expr file: " << filename << "\n";
                        size_t fileSize = 0;
                        const char* contents = HAL_JSON::ReadFileToMutableBuffer(filename.c_str(), fileSize);
                        if (contents == nullptr) {
                            std::cout << "Error: file empty or could not be found: " << filename << "\n";
                            return 0;
                        } 
                        HAL_JSON::ValidateExpression(contents);
                        delete[] contents;
                        return 0;
                    } else {
                        
                        std::cout << "unknown cmd argument:" << firstArg << "\n";
                        return 1;
                    }
                } else {
                    filename = firstArg;
                }
            } else {
                filename = argv[1];
            }
            
        } else {
            filename = "ruleset.txt";
            std::cout << "No file provided. Using default: " << filename << "\n";
        }
        std::cout << "Using provided rule file: " << filename << "\n";
        
        if (HAL_JSON::ReadAndParseRuleSetFile(filename.c_str())) {
            // maybe do something here
        } else {
            // maybe do something here
        }        


        const char* input[] = { "3", "+", "4", "*", "(", "2", "-", "1", "+", "2" };
        const char* rpn[32];
        int rpnCount = 0;

        if (HAL_JSON::ToRPN(input, 9, rpn, rpnCount)) {
            for (int i = 0; i < rpnCount; ++i) {
                printf("%s ", rpn[i]);
            }
            printf("\n");
        } else {
            printf("Error parsing expression.\n");
        }

        std::thread cmdThread(commandLoop); // start command input thread from commandLoop that is in commandLoop.h
        std::cout << "****** Starting REST api server:\n";
        HAL_JSON::REST::setup(myCallback); // this will start the server
        std::cout << "****** Init HAL_JSON Manager\n";
        HAL_JSON::Manager::setup();
        while (running) { // running is in commandLoop.h
            HAL_JSON::Manager::loop();
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        cmdThread.join(); // wait for command thread to finish
        std::cout << "Exited cleanly.\n";
        return 0;
    }