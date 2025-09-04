
#include "HAL_JSON_RULE_Engine_StatementBlock.h"
#include "HAL_JSON_RULE_Engine_IfStatement.h"

namespace HAL_JSON {
    namespace Rules {

        StatementBlock::StatementBlock () : context(nullptr), handler(nullptr), deleter(nullptr) { }

        StatementBlock::~StatementBlock ()
        {
            if (deleter) { deleter(context); context=nullptr; }
        }

        HALOperationResult dummyActionHandler(void* context) {
            return HALOperationResult::Success;
        } 

        void StatementBlock::Set(Tokens& tokens)
        {
            printf("(%d) StatementBlock::Set----------------- : %s\n",tokens.currIndex, tokens.Current().ToString().c_str());
            // note to myself, this is one statement consume only

            Token& token = tokens.Current();

            if (token.type == TokenType::If) {
                printf("---- StatementBlock::Set was if\n");
                context = new IfStatement(tokens);
                handler = IfStatement::Handler;
                deleter = DeleteAs<IfStatement>;
            } else if (token.type == TokenType::Action) {
                tokens.currentEndIndex = tokens.currIndex + token.itemsInBlock;
                printf("--------------------- StatementBlock::Set was action: %s\n",tokens.SliceToString().c_str());
                tokens.currentEndIndex = tokens.count;
                tokens.currIndex += token.itemsInBlock; // just skip it here
                //context = new ActionStatement(tokens, handler);
                
                //deleter = DeleteAs<ActionStatement>;
                handler = dummyActionHandler;
            }
            else {
                ReportTokenError(token, "StatementBlock::Set !!!! very big issue found unknown type:", TokenTypeToString(token.type));
            }
            
        }

    }
}