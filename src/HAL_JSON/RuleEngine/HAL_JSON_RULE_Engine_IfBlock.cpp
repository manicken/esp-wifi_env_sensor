
#include "HAL_JSON_RULE_Engine_IfBlock.h"

namespace HAL_JSON {
    namespace Rules {

        BranchBlock::BranchBlock()
        {

        }

        BranchBlock::~BranchBlock()
        {
            
        }

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