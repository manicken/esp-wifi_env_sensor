
#include "HAL_JSON_RULE_Engine_StatementBlock.h"
#include "HAL_JSON_RULE_Engine_IfBlock.h"

namespace HAL_JSON {
    namespace Rules {

        StatementBlock::StatementBlock () : context(nullptr), handler(nullptr), deleter(nullptr) { }

        StatementBlock::~StatementBlock ()
        {
            if (deleter) { deleter(context); context=nullptr; }
        }

        void StatementBlock::Set(Tokens& tokens) {
            Token& token = tokens.items[tokens.currIndex];
            if (token.type == TokenType::If) {
                context = new IfStatement(tokens);
                handler = IfStatement::Handler;
                deleter = DeleteAs<IfStatement>;
            } else if (token.type == TokenType::Action) {
                context = new ActionStatement(tokens);
                handler = ActionStatement::Handler;
                deleter = DeleteAs<ActionStatement>;
            } else {
                ReportTokenError(token, "StatementBlock::Set !!!! very big issue found unknown type:", TokenTypeToString(token.type));
            }
        }

    }
}