
#include "HAL_JSON_RULE_Engine_Script.h"

#define HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS

namespace HAL_JSON {
    namespace Rule {
       
        TriggerBlock::TriggerBlock() {

        }
        TriggerBlock::~TriggerBlock() {
            delete statementBlocks;
            statementBlocks = nullptr;
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

        bool ScriptsBlock::ValidateAllActiveScripts()
        {
            

            
            return true;
        }

        bool ScriptsBlock::LoadAllActiveScripts()
        {
            delete[] ScriptsBlock::scriptBlocks;
            ScriptsBlock::scriptBlocks = nullptr;
            ScriptsBlock::scriptBlocksCount = 0;

            
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