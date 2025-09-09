
#pragma once
#include <Arduino.h>
#include "../HAL_JSON_Device.h" // HALOperationResult
#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_LogicExecNode.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"

#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {

        // Forward declare StatementBlock
        struct StatementBlock;

        /** collection of StatementBlock(s) */
        struct BranchBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(BranchBlock);

            StatementBlock* items;
            int itemsCount;

            /** used to execute all StatementBlock Items one after annother */
            HALOperationResult Exec(void);

            BranchBlock();
            ~BranchBlock();
        };

        struct ConditionalBranch : public BranchBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(ConditionalBranch);

            /** is either LogicExecNode or CalcCompareRPN (pure without logic) */
            void* context;
            /** used to delete the context depending on type */
            Deleter deleter;
            HALOperationResult (*handler)(void* context);

            void Set(Tokens& tokens);

            ConditionalBranch();
            ~ConditionalBranch();
        };

        /** 
         * this is a kind of of ConditionalBranch where there are not any LogicalExpressionRPNToken list
         * there is only one instance of it inside a IfBlock item
         */
        struct UnconditionalBranch : public BranchBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(UnconditionalBranch);

            UnconditionalBranch(Tokens& tokens);
            ~UnconditionalBranch();
        };

        /** 
         * contains collections of ConditionalBranch and one optional UnconditionalBranch at the end
         * for example:
         * if <condition> then (ConditionalBranch)
         * 
         * elseif <condition> then (ConditionalBranch)
         * 
         * else (UnconditionalBranch)
         * 
         * endif
         * 
         * else here just contain a collection of OpBlock items
         */
        struct IfStatement
        {
            HAL_JSON_NOCOPY_NOMOVE(IfStatement);

            ConditionalBranch* branchItems;
            int branchItemsCount;
            /** set when there are a else branch defined, else it's set to nullptr */
            UnconditionalBranch* elseBranch; 

            IfStatement(Tokens& tokens);
            ~IfStatement();

            static HALOperationResult Handler(void* context);
        };

    }
}