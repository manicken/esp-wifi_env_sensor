
#pragma once
#include <Arduino.h>
#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Engine_LogicalExpressionRPNToken.h"
#include "HAL_JSON_RULE_Engine_StatementBlock_s.h"

namespace HAL_JSON {
    namespace Rule {

        struct ConditionalBranch : public StatementBlocks
        {
            HAL_JSON_NOCOPY_NOMOVE(ConditionalBranch);

            LogicalExpressionRPNToken* expressionTokens;
            int expressionTokensCount;

            /** the context that is passed into this is of type ConditionalBranch */
            static bool ShouldExec(void* context);

            ConditionalBranch();
            ~ConditionalBranch();
        };

        /** 
         * this is a kind of of ConditionalBranch where there are not any LogicalExpressionRPNToken list
         * and where the function ShouldExec allways return true
         */
        struct UnconditionalBranch : public StatementBlocks
        {
            HAL_JSON_NOCOPY_NOMOVE(UnconditionalBranch);

            /** this will allways return true */
            static bool ShouldExec(void* context);

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
            HAL_JSON_NOCOPY_NOMOVE(IfBlock);

            StatementBlocks* branchItems;
            int branchItemsCount;

            

            IfBlock();
            ~IfBlock();
        };

    }
}