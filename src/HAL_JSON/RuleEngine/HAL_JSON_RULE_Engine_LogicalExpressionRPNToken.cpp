
#include "HAL_JSON_RULE_Engine_LogicalExpressionRPNToken.h"

namespace HAL_JSON {
    namespace Rules {
        RPNStack<bool> boolValueStack;

        LogicalExpressionRPNToken::LogicalExpressionRPNToken() {

        }

        LogicalExpressionRPNToken::~LogicalExpressionRPNToken() {
            
        }
    }
}