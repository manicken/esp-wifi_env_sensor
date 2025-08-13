
#include "HAL_JSON_RULE_Engine_IfBlock.h"

namespace HAL_JSON {
    namespace Rule {

        ConditionalBranch::ConditionalBranch()
        {

        }
        ConditionalBranch::~ConditionalBranch()
        {

        }

        UnconditionalBranch::UnconditionalBranch()
        {

        }
        UnconditionalBranch::~UnconditionalBranch()
        {

        }

        bool UnconditionalBranch::ShouldExec(void* context)
        {
            return true;
        }

        IfBlock::IfBlock()
        {

        }
        IfBlock::~IfBlock()
        {
            
        }
    }
}