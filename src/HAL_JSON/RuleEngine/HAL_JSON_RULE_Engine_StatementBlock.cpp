
#include "HAL_JSON_RULE_Engine_StatementBlock.h"

namespace HAL_JSON {
    namespace Rule {

        StatementBlock::StatementBlock () : context(nullptr), handler(nullptr), deleter(nullptr) { }

        StatementBlock::~StatementBlock ()
        {
            if (deleter) deleter(context);
        }

        

    }
}