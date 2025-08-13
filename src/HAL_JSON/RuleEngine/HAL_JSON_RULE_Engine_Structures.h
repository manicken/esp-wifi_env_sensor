
#pragma once

#include <Arduino.h>
#include "HAL_JSON_RUL_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_LogicalExpressionRPNToken.h"
#include "../HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {
    namespace Rule {
        namespace Structures {

            /** OpBlock contains either a IfBlock or a ExecBlock */
            struct OpBlock
            {
                void* context;
                HALOperationResult (*handler)(void* context);
                Deleter deleter;
 
                OpBlock(OpBlock&) = delete;          // no copy constructor
                OpBlock& operator=(const OpBlock&) = delete; // no copy assignment
                OpBlock(OpBlock&& other) = delete;           // no move constructor
                OpBlock& operator=(OpBlock&& other) = delete; // no move assignment

                OpBlock();
                ~OpBlock();
            };

            /** collection of OpBlock(s) */
            struct OpBlocks
            {
                OpBlock* opItems;
                int opItemsCount;

                /** used to execute all opItems one after annother */
                HALOperationResult (*Exec)(void);

                /** returns true if exec should run */
                bool (*ShouldExec)(void);

                OpBlocks(OpBlocks&) = delete;          // no copy constructor
                OpBlocks& operator=(const OpBlocks&) = delete; // no copy assignment
                OpBlocks(OpBlocks&& other) = delete;           // no move constructor
                OpBlocks& operator=(OpBlocks&& other) = delete; // no move assignment

                OpBlocks();
                ~OpBlocks();
            };

            struct ExecBlock
            {
                CachedDeviceAccess* target;
                CalcRPNToken* sourceCalcItems;
                int sourceCalcItemsCount;

                ExecBlock(ExecBlock&) = delete;          // no copy constructor
                ExecBlock& operator=(const ExecBlock&) = delete; // no copy assignment
                ExecBlock(ExecBlock&& other) = delete;           // no move constructor
                ExecBlock& operator=(ExecBlock&& other) = delete; // no move assignment

                ExecBlock();
                ~ExecBlock();
            };

            struct ConditionalBranch : public OpBlocks
            {
                LogicalExpressionRPNToken* expressionTokens;
                int expressionTokensCount;

                /** the context that is passed into this is of type ConditionalBranch */
                static bool ShouldExec(void* context);

                ConditionalBranch(ConditionalBranch&) = delete;          // no copy constructor
                ConditionalBranch& operator=(const ConditionalBranch&) = delete; // no copy assignment
                ConditionalBranch(ConditionalBranch&& other) = delete;           // no move constructor
                ConditionalBranch& operator=(ConditionalBranch&& other) = delete; // no move assignment

                ConditionalBranch();
                ~ConditionalBranch();
            };

            /** 
             * this is a kind of of ConditionalBranch where there are not any LogicalExpressionRPNToken list
             * and where the function ShouldExec allways return true
             */
            struct UnconditionalBranch : public OpBlocks
            {
                /** this will allways return true */
                static bool ShouldExec(void* context);

                UnconditionalBranch(UnconditionalBranch&) = delete;          // no copy constructor
                UnconditionalBranch& operator=(const UnconditionalBranch&) = delete; // no copy assignment
                UnconditionalBranch(UnconditionalBranch&& other) = delete;           // no move constructor
                UnconditionalBranch& operator=(UnconditionalBranch&& other) = delete; // no move assignment

                UnconditionalBranch();
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
            struct IfBlock
            {
                OpBlocks* branchItems;
                int branchItemsCount;

                IfBlock(IfBlock&) = delete;          // no copy constructor
                IfBlock& operator=(const IfBlock&) = delete; // no copy assignment
                IfBlock(IfBlock&& other) = delete;           // no move constructor
                IfBlock& operator=(IfBlock&& other) = delete; // no move assignment

                IfBlock();
                ~IfBlock();
            };

            /**
             * A single trigger definition at the root level of a script.
             * Each trigger is linked to one or more executable operation blocks.
             */
            struct TriggerBlock
            {
                bool (*triggerSource)();
                //bool (*triggerSource)(void* context);
                //void* context;  // optional — lets the triggerSource read its own state
                OpBlocks* triggerExec;
            };

            /**
             * Represents a single script file.
             * A script may contain one or more top-level triggers.
             */
            struct ScriptBlock
            {
                TriggerBlock* triggerBlocks;
                int triggerBlockCount;
            };

            /**
             * Global container for all loaded scripts in the engine.
             * This is the highest-level structure in the script engine hierarchy.
             */
            struct ScriptsBlock
            {
                static ScriptBlock* scriptBlocks;
                static int scriptBlocksCount;
            };

        }
    }
}