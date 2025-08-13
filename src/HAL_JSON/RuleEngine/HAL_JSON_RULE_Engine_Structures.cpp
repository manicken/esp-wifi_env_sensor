
#include "HAL_JSON_RULE_Engine_Structures.h"

#define HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS

namespace HAL_JSON {
    namespace Rule {
        namespace Structures {
            

            OpBlock::OpBlock() : context(nullptr), handler(nullptr), deleter(nullptr) { }

            OpBlock::~OpBlock()
            {
                if (deleter) deleter(context);
            }

            bool UnconditionalBranch::ShouldExec(void* context) {
                return true;
            }
            
        }
    }
}