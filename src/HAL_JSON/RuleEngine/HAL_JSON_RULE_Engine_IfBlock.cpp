
#include "HAL_JSON_RULE_Engine_IfBlock.h"
#include "HAL_JSON_RULE_Engine_StatementBlock.h"

namespace HAL_JSON {
    namespace Rules {

        BranchBlock::BranchBlock()
        {

        }

        BranchBlock::~BranchBlock()
        {
            
        }

        HALOperationResult BranchBlock::Exec()
        {

        }

        ConditionalBranch::ConditionalBranch()
        {

        }
        ConditionalBranch::~ConditionalBranch()
        {

        }
        void ConditionalBranch::Set(Tokens& tokens)
        {

        }
        bool ConditionalBranch::ShouldExec()
        {
            return true;
        }

        UnconditionalBranch::UnconditionalBranch(Tokens& tokens)
        {

        }
        UnconditionalBranch::~UnconditionalBranch()
        {

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

        }
    }
}