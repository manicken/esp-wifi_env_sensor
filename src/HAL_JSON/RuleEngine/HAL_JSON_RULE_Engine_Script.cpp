
#include "HAL_JSON_RULE_Engine_Script.h"


#define HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS

namespace HAL_JSON {
    namespace Rules {
       
        TriggerBlock::TriggerBlock() {

        }
        TriggerBlock::~TriggerBlock() {
            delete[] items;
            items = nullptr;
            itemsCount = 0;
        }
        bool TriggerBlock::AllwaysRun() {
            return true;
        }

        ScriptBlock::ScriptBlock()
        {

        }
        ScriptBlock::~ScriptBlock()
        {
            delete[] triggerBlocks;
        }

        
        ScriptBlock* ScriptsBlock::scriptBlocks = nullptr;
        int ScriptsBlock::scriptBlocksCount = 0;
        int ScriptsBlock::currentScriptIndex = 0;

        bool ScriptsBlock::ValidateAllActiveScripts()
        {
            Rules::Parser::ReadAndParseRuleSetFile("rules1.txt", nullptr);
            return true;
        }

        void ScriptsBlock::ScriptFileParsed(Tokens& tokens) {

            ScriptBlock& script = scriptBlocks[currentScriptIndex];
            // here script.Set should be used and pass in tokens
            // that way it's the beginning of separation into the multiple structs

            script.triggerBlockCount = tokens.rootBlockCount;
            script.triggerBlocks = new TriggerBlock[tokens.rootBlockCount];
            int currTriggerBlockIndex = 0;
            tokens.currIndex = 0;
            while (tokens.currIndex < tokens.count) {
                Token& token = tokens.items[tokens.currIndex];
                
                if (token.type == TokenType::On)
                {
                    TriggerBlock& triggerBlock = script.triggerBlocks[currTriggerBlockIndex++];

                    // set allways run for now
                    triggerBlock.triggerSource = TriggerBlock::AllwaysRun;
                    // a on statement can contain multiple blocks
                    // however the 'on' token don't contain the actual itemcount
                    // it's the 'then' token that follows
                    // so i need to find the following 'then' token
                    // by some helper function
                    // but as 'on'/triggers is defined in one string without spaces
                    // it will just be the next next token
                    triggerBlock.items = new StatementBlock[tokens.items[tokens.currIndex+2].itemsInBlock];
                    triggerBlock.itemsCount = token.itemsInBlock;
                }
                else if (token.type == TokenType::If)
                {
                    TriggerBlock& triggerBlock = script.triggerBlocks[currTriggerBlockIndex++];

                    triggerBlock.triggerSource = TriggerBlock::AllwaysRun;
                    // a if block at root only contain one item
                    // which is itself only
                    triggerBlock.items = new StatementBlock[1];
                    triggerBlock.itemsCount = 1;

                }
                else
                    tokens.currIndex++;
            }

        }

        bool ScriptsBlock::LoadAllActiveScripts()
        {
            delete[] scriptBlocks;
            scriptBlocks = nullptr;
            scriptBlocksCount = 0;

            // here i will load the active scripts file and parse which scripts to load
            // and how many to load
            // currently for development first test we only load one file
            
            int count = 1; // set to 1 for development test
            scriptBlocks = new ScriptBlock[count];
            scriptBlocksCount = count;

            for (int i = 0;i<count;i++) {
                currentScriptIndex = i;
                // this should now pass and execute the given callback
                Rules::Parser::ReadAndParseRuleSetFile("rules1.txt", ScriptFileParsed);
            }
            return true;
        }

        bool ScriptsBlock::ValidateAndLoadAllActiveScripts()
        {
            if (ValidateAllActiveScripts() == false) return false;
            if (LoadAllActiveScripts() == false) {
#ifdef _WIN32
#include <iostream>
                std::cout << "Serious problem could not load scripts\n";
#endif
                return false;
            }
            return true;
        }

    }
}