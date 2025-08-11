
#include "HAL_JSON_RULE_Engine_Structures.h"

namespace HAL_JSON {
    namespace Rule {
        namespace Structures {

            OpBlock::OpBlock() : type(OpBlockType::NotSet) 
            {
                unset = nullptr;
            }

            OpBlock::~OpBlock()
            {
                if (type == OpBlockType::If) {
                    ifData.~IfData();
                } else if (type == OpBlockType::Exec) {
                    execData.~ExecData();
                } // else its unset so nothing needs freeing
            }

            OpBlock OpBlock::MakeIf(const void* cond, uint16_t firstChild, uint16_t count)
            {
                OpBlock block;
                block.type = OpBlockType::If;
                block.ifData = { cond, firstChild, count };
                return block;
            }

            OpBlock OpBlock::MakeExec(const void* func, uint16_t a1, uint16_t a2)
            {
                OpBlock block;
                block.type = OpBlockType::Exec;
                block.execData = { func, a1, a2 };
                return block;
            }

        }
    }
}