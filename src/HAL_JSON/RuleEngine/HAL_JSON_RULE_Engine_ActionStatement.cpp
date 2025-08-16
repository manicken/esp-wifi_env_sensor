
#include "HAL_JSON_RULE_Engine_ActionStatement.h"

namespace HAL_JSON {
    namespace Rules {

        ActionStatement::ActionStatement(Tokens& tokens)
        {
            Token& token = tokens.Current(); // this now point to the action-type token
        }
        ActionStatement::~ActionStatement()
        {

        }

        HALOperationResult ActionStatement::Handler(void* context) {

        }
        
    }
}