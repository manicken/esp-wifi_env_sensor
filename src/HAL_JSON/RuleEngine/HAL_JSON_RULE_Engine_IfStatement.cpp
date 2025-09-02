
#include "HAL_JSON_RULE_Engine_IfStatement.h"
#include "HAL_JSON_RULE_Engine_StatementBlock.h"

namespace HAL_JSON {
    namespace Rules {

        BranchBlock::BranchBlock()
        {

        }

        BranchBlock::~BranchBlock()
        {
            delete[] items;
        }

        HALOperationResult BranchBlock::Exec()
        {
            return HALOperationResult::Success;
        }

        ConditionalBranch::ConditionalBranch()
        {

        }
        ConditionalBranch::~ConditionalBranch()
        {
            //delete[] items; is deleted by BranchBlock destructor
        }
        void ConditionalBranch::Set(Tokens& tokens)
        {
            // first increment to next and then get
            const Token& expression = tokens.items[++tokens.currIndex];

            // the following consumes the tokens
            ExpressionTokens* expTokens = Expressions::GenerateRPNTokens(tokens);
            // builds the temporary tree using memory pool
            LogicRPNNode* lrpnNode = Expressions::BuildLogicTree(expTokens);

            // TODO make function that builds the IF condition exec structure


            //when consumed we are at the then
            Token& thenToken = tokens.items[tokens.currIndex++]; // get and consume
            // here extract the itemsCount
            itemsCount = thenToken.itemsInBlock;
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                items[i].Set(tokens);
            }
        }

        UnconditionalBranch::UnconditionalBranch(Tokens& tokens)
        {
            const Token& elseToken = tokens.items[tokens.currIndex++]; // get and consume

            itemsCount = elseToken.itemsInBlock;
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                items[i].Set(tokens);
            }
        }
        UnconditionalBranch::~UnconditionalBranch()
        {
            //delete[] items; is deleted by BranchBlock destructor
        }

        IfStatement::IfStatement(Tokens& tokens)
        {
            Token& ifToken = tokens.Current(); // this now points to the if-type token
            branchItemsCount = ifToken.itemsInBlock;
            if (ifToken.hasElse == 1) branchItemsCount--; // minus one as the else case is handled separately
            branchItems = new ConditionalBranch[branchItemsCount];
            // allways consume the first If condition
            branchItems[0].Set(tokens);

            for (int i=1;i<branchItemsCount;i++) {
                Token& token = tokens.Current();

                if (token.type == TokenType::ElseIf) {
                    // this will consume all tokens until it find either annother ElseIf Else or a EndIf (it will also consume cascaded ifblocks accordingly)
                    branchItems[i].Set(tokens); 
                } else if (token.type == TokenType::Else) {
                    // will consume all tokens until a EndIf is found (it will also consume cascaded ifblocks accordingly)
                    elseBranch = new UnconditionalBranch(tokens);
                } 
                // nothing else is needed here as at this stage it's guaranteed that the token 'stream' is validated
            }
        }
        IfStatement::~IfStatement()
        {
            delete[] branchItems;
            delete elseBranch;
        }

        HALOperationResult IfStatement::Handler(void* context) {
            IfStatement* ifStatement = static_cast<IfStatement*>(context);
            bool anyDidExec = false;
            int ifStatementBranchItemsCount = ifStatement->branchItemsCount;
            ConditionalBranch* ifStatementBranchItems = ifStatement->branchItems;
            HALOperationResult res;
            for (int i=0;i<ifStatementBranchItemsCount;i++) {
                HALOperationResult res = ifStatementBranchItems[i].handler(&ifStatementBranchItems[i]);
                if (res == HALOperationResult::IfConditionTrue) {
                    anyDidExec = true;
                    res = ifStatementBranchItems[i].Exec();
                    break;
                } else if (res != HALOperationResult::IfConditionFalse) {
                    return res; // direct fail stop exec here??
                }
            }
            if (anyDidExec == false && ifStatement->elseBranch != nullptr) {
                res = ifStatement->elseBranch->Exec();
            }
            return res;
        }
    }
}