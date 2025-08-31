
#include "HAL_JSON_RULE_Engine_LogicExecNode.h"

namespace HAL_JSON {
    namespace Rules {
        //LogicRPNNode::LogicRPNNode()
        //    : calcRPN(nullptr), childA(nullptr), childB(nullptr), op(nullptr)/*, type(OpType::Invalid)*/ {}

        LogicRPNNode::LogicRPNNode(ExpressionTokens* tokens, int startIndex, int endIndex) {
            // the following will later be replaced by the actual object creation
            int calcRPNLength = endIndex-startIndex;
            calcRPN = new ExpressionTokens(calcRPNLength);
            ExpressionToken* tokenItems = tokens->items;
            ExpressionToken* calcRPNitems = calcRPN->items;
            for (int i=0;i<calcRPNLength;i++) {
                calcRPNitems[i] = tokenItems[startIndex++];
            }
            childA = nullptr;
            childB = nullptr;
            op = nullptr;
        }
        LogicRPNNode::LogicRPNNode(ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs) {
            calcRPN = nullptr;
            childA = lhs;
            childB = rhs;
            op = opTok;
        }

        LogicRPNNode::~LogicRPNNode() {
            delete calcRPN;
            delete childA;
            delete childB;
        }
    }
}