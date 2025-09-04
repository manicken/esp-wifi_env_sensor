
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
            if (deleter && context) {
                deleter(context);
                context = nullptr;
            }
            //delete[] items; is deleted by BranchBlock destructor

        }
        void ConditionalBranch::Set(Tokens& tokens)
        {
            printf("(%d) ConditionalBranch::Set(Tokens& tokens): %s\n", tokens.currIndex, tokens.Current().ToString().c_str());
            // consume the If / ElseIf token itself
            tokens.currIndex++;
            // set the slice to the items of this expression
            tokens.currentEndIndex = tokens.currIndex + tokens.Current().itemsInBlock;
            tokens.firstTokenStartOffset = nullptr;
            // the following consumes the expression tokens
            ExpressionTokens* expTokens = Expressions::GenerateRPNTokens(tokens); // note here. expTokens is non owned
            printf("ConditionalBranch::Set expTokens:\n%s\n", PrintExpressionTokens(*expTokens, 0, expTokens->currentCount).c_str());
            // restore the slice to full token array
            tokens.currentEndIndex = tokens.count;
            // builds the temporary tree using memory pool
            LogicRPNNode* lrpnNode = Expressions::BuildLogicTree(expTokens); // note here. lrpnNode is non owned 

            if (lrpnNode->calcRPNStartIndex != -1) { // pure calc compare expression, no logic
                context = new CalcRPN(expTokens, lrpnNode->calcRPNStartIndex, lrpnNode->calcRPNEndIndex);
                handler = &LogicExecNode::Eval_Calc;
                deleter = DeleteAs<CalcRPN>;
            } else {
                LogicExecNode* newExecNode = new LogicExecNode(expTokens, lrpnNode);
                context = newExecNode;
                deleter = DeleteAs<LogicExecNode>;
                handler = newExecNode->handler;
            }

            //when consumed we are at the then
            Token& thenToken = tokens.items[tokens.currIndex++]; // get and consume
            if (thenToken.type != TokenType::Then) {
                ReportTokenError(thenToken, " ----------------------------- is not a then token: ");
            }
            // here extract the itemsCount
            itemsCount = thenToken.itemsInBlock;
            printf("(%d) ---------------------------- then item count:%d %s\n",tokens.currIndex, itemsCount, tokens.Current().ToString().c_str());
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                if (tokens.SkipIgnores() == false) break;
                TokenType currTokenType = tokens.Current().type;
                if (currTokenType == TokenType::EndIf || currTokenType == TokenType::ElseIf || currTokenType == TokenType::Else)
                    break;
                items[i].Set(tokens); // each call should consume all tokens
            }
        }

        UnconditionalBranch::UnconditionalBranch(Tokens& tokens)
        {
            printf("(%d) UnconditionalBranch::UnconditionalBranch: %s\n", tokens.currIndex, tokens.Current().ToString().c_str());
            const Token& elseToken = tokens.items[tokens.currIndex++]; // get and consume

            itemsCount = elseToken.itemsInBlock;
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                if (tokens.SkipIgnores() == false) break;
                TokenType currTokenType = tokens.Current().type;
                if (currTokenType == TokenType::EndIf || currTokenType == TokenType::ElseIf || currTokenType == TokenType::Else)
                    break;
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
            if (ifToken.type != TokenType::If) { printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ifToken.type != TokenType::If\n");}
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
                } else if (token.type == TokenType::EndIf)
                    break;
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