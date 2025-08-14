
#include "HAL_JSON_RULE_Engine_Script.h"
#include "HAL_JSON_RULE_Parser.h"

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

        void ScriptFileParsed(Tokens& tokens) {

            ScriptsBlock::scriptBlocks[ScriptsBlock::currentScriptIndex].triggerBlockCount = tokens.rootBlockCount;
            ScriptsBlock::scriptBlocks[ScriptsBlock::currentScriptIndex].triggerBlocks = new TriggerBlock[tokens.rootBlockCount];


        }

        bool ScriptsBlock::LoadAllActiveScripts()
        {
            delete[] ScriptsBlock::scriptBlocks;
            ScriptsBlock::scriptBlocks = nullptr;
            ScriptsBlock::scriptBlocksCount = 0;

            // here i will load the active scripts file and parse which scripts to load
            // and how many to load
            // currently for development first test we only load one file
            
            int count = 1; // set to 1 for development test
            ScriptsBlock::scriptBlocks = new ScriptBlock[count];
            ScriptsBlock::scriptBlocksCount = count;

            for (int i = 0;i<count;i++) {
                ScriptsBlock::currentScriptIndex = i;
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