
#include "HAL_JSON_RULE_Engine_LogicExecNode.h"

namespace HAL_JSON {
    namespace Rules {

        LogicExecNode::LogicExecNode(LogicExecNode* lhs, LogicExecNode* rhs) {

        }

        HALOperationResult LogicExecNode::EvalAnd_LL(LogicExecNode* context) {  // LL (logic logic)
            return HALOperationResult::Success;
        }
        HALOperationResult LogicExecNode::EvalAnd_LC(LogicExecNode* context) { // LC (logic calc)
            return HALOperationResult::Success;
        }
        HALOperationResult LogicExecNode::EvalAnd_CL(LogicExecNode* context) { // CL (calc logic)
            return HALOperationResult::Success;
        }
        HALOperationResult LogicExecNode::EvalAnd_CC(LogicExecNode* context) { // CC (calc calc)
            return HALOperationResult::Success;
        }

        HALOperationResult LogicExecNode::EvalOr_LL(LogicExecNode* context) { // LL
            return HALOperationResult::Success;
        }
        HALOperationResult LogicExecNode::EvalOr_LC(LogicExecNode* context) { // LC
            return HALOperationResult::Success;
        }
        HALOperationResult LogicExecNode::EvalOr_CL(LogicExecNode* context) { // CL
            return HALOperationResult::Success;
        }
        HALOperationResult LogicExecNode::EvalOr_CC(LogicExecNode* context) { // CC
            return HALOperationResult::Success;
        }

        LogicRPNNode::LogicRPNNode()
            : calcRPNStartIndex(-1),calcRPNEndIndex(-1), childA(nullptr), childB(nullptr), op(nullptr) {}

        void LogicRPNNode::Set(ExpressionTokens* tokens, int startIndex, int endIndex) {
            calcRPNStartIndex = startIndex;
            calcRPNEndIndex = endIndex;
            childA = nullptr;
            childB = nullptr;
            op = nullptr;
        }

        void LogicRPNNode::Set(ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs) {
            calcRPNStartIndex = -1;
            calcRPNEndIndex = -1;
            childA = lhs;
            childB = rhs;
            op = opTok;
        }

        LogicRPNNode::~LogicRPNNode() {
            // Do nothing — memory is managed by the pool
            childA = nullptr;
            childB = nullptr;
            op = nullptr;
        }
    }
}