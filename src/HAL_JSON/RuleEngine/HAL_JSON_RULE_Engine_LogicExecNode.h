#pragma once

#include "../HAL_JSON_Device.h" // HALOperationResult
#include "HAL_JSON_RULE_Engine_Support.h" // Deleter
#include "HAL_JSON_RULE_Expression_Token.h" // ExpressionTokens, ExpressionToken

namespace HAL_JSON {
    namespace Rules {

        struct LogicExecNode {
            void* childA;   // either LogicExecNode* or CalcRPNContext*
            void* childB;   // either LogicExecNode* or CalcRPNContext*
            HALOperationResult (*handler)(LogicExecNode* ctx);  // function pointer to EvalAnd/Or variant
            Deleter2x deleter; // handler that delete the childs depending on set types

            LogicExecNode(LogicExecNode* lhs, LogicExecNode* rhs);

            static HALOperationResult EvalAnd_LL(LogicExecNode* context);  // LL (logic logic)
            static HALOperationResult EvalAnd_LC(LogicExecNode* context); // LC (logic calc)
            static HALOperationResult EvalAnd_CL(LogicExecNode* context); // CL (calc logic)
            static HALOperationResult EvalAnd_CC(LogicExecNode* context); // CC (calc calc)

            static HALOperationResult EvalOr_LL(LogicExecNode* context);    // LL
            static HALOperationResult EvalOr_LC(LogicExecNode* context);  // LC
            static HALOperationResult EvalOr_CL(LogicExecNode* context);  // CL
            static HALOperationResult EvalOr_CC(LogicExecNode* context); // CC
        };

        

        /** only both for development tests and as a temp structure*/
        struct LogicRPNNode {

            //ExpressionTokens* calcRPN;  // leaf if this i non nullptr
            int calcRPNStartIndex; // leaf if this is not -1
            int calcRPNEndIndex;
            /** is owned by Expressions "static" class */
            LogicRPNNode* childA;   // nested nodes, nullptr when leaf
            /** is owned by Expressions "static" class */
            LogicRPNNode* childB;   // nested nodes, nullptr when leaf
            /** is owned by Expressions "static" class */
            ExpressionToken* op;                     // "&&" or "||", nullptr when leaf

            LogicRPNNode();
            void Set(ExpressionTokens* tokens, int startIndex, int endIndex);
            void Set(ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs);
            ~LogicRPNNode();
        };
    }
}