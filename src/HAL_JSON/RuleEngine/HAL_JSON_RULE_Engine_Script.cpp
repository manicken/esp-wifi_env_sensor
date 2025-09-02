
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
        bool TriggerBlock::AllwaysRun(void* context) {
            return true;
        }

        void TriggerBlock::Set(int _itemsCount, Tokens& tokens) {
            items = new StatementBlock[_itemsCount];
            itemsCount = _itemsCount;

            for (int i=0;i<_itemsCount;i++) {
                items[i].Set(tokens);
            }
        }

        ScriptBlock::ScriptBlock()
        {

        }
        ScriptBlock::~ScriptBlock()
        {
            delete[] triggerBlocks;
        }

        void ScriptBlock::Set(Tokens& tokens) {
            
            triggerBlockCount = tokens.rootBlockCount;
            triggerBlocks = new TriggerBlock[tokens.rootBlockCount];
            
            tokens.currIndex = 0;

            for (int i = 0; i < tokens.rootBlockCount || tokens.currIndex < tokens.count; i++) { // tokens.currIndex is incremented elsewhere and is included here as a out of bounds failsafe
                TriggerBlock& triggerBlock = triggerBlocks[i];
                Token& token = tokens.Current();
                
                if (token.type == TokenType::On)
                {
                    tokens.currIndex++;
                    
                    Token& triggerSourceToken = tokens.items[tokens.currIndex++];
                    if (triggerSourceToken.EqualsIC("eachloop"))
                        triggerBlock.triggerSource = TriggerBlock::AllwaysRun;
                    else
                    {
                        // set allways run for now
                        // TODO implement other sources
                        // if not reserved trigger sources then try to get it as a device based trigger
                        // this should allready be checked in the validate script part
                        // so accessing it here should allways pass
                        triggerBlock.triggerSource = TriggerBlock::AllwaysRun;
                    }
                    triggerBlock.Set(tokens.items[tokens.currIndex++].itemsInBlock, tokens);
                }
                else if (token.type == TokenType::If)
                {
                    // wrap root-level if into a trigger block that always runs
                    triggerBlock.triggerSource = TriggerBlock::AllwaysRun;
                    triggerBlock.Set(1, tokens);
                }
                else
                    tokens.currIndex++;
            }
        }

        
        ScriptBlock* ScriptsBlock::scriptBlocks = nullptr;
        int ScriptsBlock::scriptBlocksCount = 0;
        int ScriptsBlock::currentScriptIndex = 0;

        bool ScriptsBlock::ValidateAllActiveScripts()
        {
            Rules::Expressions::CalcStackSizesInit();

            Rules::Parser::ReadAndParseRuleSetFile("rules1.txt", nullptr);
            return true;
        }

        void ScriptsBlock::ScriptFileParsed(Tokens& tokens) {
            scriptBlocks[currentScriptIndex].Set(tokens);
        }

        bool ScriptsBlock::LoadAllActiveScripts()
        {
            delete[] scriptBlocks;
            scriptBlocks = nullptr;
            scriptBlocksCount = 0;
            Rules::Expressions::InitStacks();
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