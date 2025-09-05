
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
            //printf("(%d) TriggerBlock::Set----------------- triggerblock statement item count:%d %s\n",tokens.currIndex, _itemsCount, tokens.Current().ToString().c_str());
            
            itemsCount = _itemsCount;
            items = new StatementBlock[_itemsCount];
            
           // printf("see if whe come her\n");
            for (int i=0;i<_itemsCount;i++) {
                if (tokens.SkipIgnores() == false) { printf("reached end\n"); break; }
                
                while (tokens.Current().type == TokenType::EndIf) // skips any endif
                    tokens.currIndex++;

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
            int i = 0;
            // i increments only when we actually assign a trigger block
            // tokens.currIndex increments for every token we process or skip
            while (i < tokens.rootBlockCount && tokens.currIndex < tokens.count) { // tokens.currIndex is incremented elsewhere and is included here as a out of bounds failsafe
                
                Token& token = tokens.Current();
                
                if (token.type == TokenType::On)
                {
                    TriggerBlock& triggerBlock = triggerBlocks[i++]; // consume a trigger block
                    printf("\n(%d) FOUND ON TOKEN\n", tokens.currIndex);
                    tokens.currIndex++; // consume the On token as it dont have any important data
                    
                    Token& triggerSourceToken = tokens.GetNextAndConsume();//.items[tokens.currIndex++]; // get and consume
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
                    //ReportTokenInfo(tokens.Current(), "this should be a then token: ", tokens.Current().ToString().c_str());
                    int itemCount = tokens.Current().itemsInBlock;
                    tokens.currIndex++; // consume the then
                    triggerBlock.Set(itemCount, tokens); // get number of items and consume
                }
                else if (token.type == TokenType::If) 
                {
                    TriggerBlock& triggerBlock = triggerBlocks[i++]; // consume a trigger block
                    printf("\n(%d) FOUND IF TOKEN\n", tokens.currIndex);
                    // here we dont consume anything just pass 
                    // wrap root-level if into a trigger block that always runs
                    triggerBlock.triggerSource = TriggerBlock::AllwaysRun; // line 83
                    triggerBlock.Set(1, tokens); // line 84
                }
                else {
                    printf("\n(%d) SKIPPING TOKEN: %s\n", tokens.currIndex, token.ToString().c_str());
                    tokens.currIndex++;
                    
                }
            }
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
            Expressions::ReportInfo("\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**                                                                                  **\n");
            Expressions::ReportInfo("** ██       ██████   █████  ██████      ███████  ██████ ██████  ██ ██████  ████████ **\n");
            Expressions::ReportInfo("** ██      ██    ██ ██   ██ ██   ██     ██      ██      ██   ██ ██ ██   ██    ██    **\n");
            Expressions::ReportInfo("** ██      ██    ██ ███████ ██   ██     ███████ ██      ██████  ██ ██████     ██    **\n");
            Expressions::ReportInfo("** ██      ██    ██ ██   ██ ██   ██          ██ ██      ██   ██ ██ ██         ██    **\n");
            Expressions::ReportInfo("** ███████  ██████  ██   ██ ██████      ███████  ██████ ██   ██ ██ ██         ██    **\n");
            Expressions::ReportInfo("**                                                                                  **\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            scriptBlocks[currentScriptIndex].Set(tokens);
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            printf("\n\ntokens.currIndex(%d) of tokens.count(%d) reached end of 'script'\n\n", tokens.currIndex, tokens.count);
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
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
            Rules::Expressions::CalcStackSizesInit();
            if (ValidateAllActiveScripts() == false) return false;
            
            Rules::Expressions::InitStacks();
            if (LoadAllActiveScripts() == false) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
                std::cout << "Serious problem could not load scripts\n";
#endif
                return false;
            }
            return true;
        }

        HALOperationResult TriggerBlock::Exec() {
            for (int i=0;i<itemsCount;i++) {
                StatementBlock& statementItem = items[i];
                if (statementItem.handler == nullptr) {
                    printf("\nERRORERRORERRORERRORERRORERRORERRORERRORERRORERROR statementItem.handler == nullptr\n");
                    break;
                }
                HALOperationResult res = statementItem.handler(statementItem.context);
                if (res != HALOperationResult::Success) {
                    return res; // direct return on any failure here
                }
            }
            return HALOperationResult::Success;
        }

        void ScriptBlock::Exec() {
            for (int i=0;i<triggerBlockCount;i++) {
                //if (triggerBlocks[i].triggerSource(triggerBlocks[i].context) == false)
                //    continue;
                HALOperationResult res = triggerBlocks[i].Exec();
                if (res != HALOperationResult::Success) {
                    GlobalLogger.Error(F("trigger: "), HALOperationResultToString(res));
//#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    printf("script exec error:%s\n", HALOperationResultToString(res));
//#endif
                }
            }
        }

        void ScriptsBlock::Exec() {
            for (int i=0;i<scriptBlocksCount;i++) {
                scriptBlocks[i].Exec();
            }
        }

    }
}