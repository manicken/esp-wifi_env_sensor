#include "HAL_JSON_RULE_Engine_Support.h"
#include "HAL_JSON_RULE_Expression_Token.h"

namespace HAL_JSON {
    namespace Rules {

        struct LogicExecNode {
            void* childA;   // either LogicExecNode* or CalcRPNContext*
            void* childB;   // either LogicExecNode* or CalcRPNContext*
            bool (*handler)(LogicExecNode* ctx);  // function pointer to EvalAnd/Or variant
            Deleter deleter; // handler that delete the childs depending on set types
        };

        /** only used for development tests */
        struct LogicRPNNode {

            ExpressionTokens* calcRPN;  // leaf if this i non nullptr
            /** is owned and needs to be deleted */
            LogicRPNNode* childA;   // nested nodes, nullptr when leaf
            /** is owned and needs to be deleted */
            LogicRPNNode* childB;   // nested nodes, nullptr when leaf
            /** this is non owned, it's owned by the input token stream */
            ExpressionToken* op;                     // "&&" or "||", nullptr when leaf

            //LogicRPNNode();
            LogicRPNNode(ExpressionTokens* tokens, int startIndex, int endIndex);
            LogicRPNNode(ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs);
            ~LogicRPNNode();
        };
    }
}