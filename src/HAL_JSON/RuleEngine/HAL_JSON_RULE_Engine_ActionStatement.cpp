
#include "HAL_JSON_RULE_Engine_ActionStatement.h"
#include "HAL_JSON_RULE_Parser.h"
#include "HAL_JSON_RULE_Engine_Operators.h"

namespace HAL_JSON {
    namespace Rules {

        ActionStatement::ActionStatement(Tokens& tokens, ActionHandler& handlerOut)
        {
            // ExtractAssignmentParts "consumes" the tokens until the next action or whatever coming after
            AssignmentParts* actionParts = Parser::ExtractAssignmentParts(tokens);
            Expressions::ReportInfo("\n**************** Action ************* lefthand side:" + actionParts->lhs.ToString() + "\n");
            Expressions::ReportInfo("**************** Action ******** assigment operator:" + std::string(1, actionParts->op) + "\n");
            
            ZeroCopyString varOperand = actionParts->lhs;
            ZeroCopyString funcName = actionParts->lhs;
            varOperand = funcName.SplitOffHead('#');
            target = new CachedDeviceAccess(varOperand, funcName);

            ExpressionTokens* expTokens = Expressions::GenerateRPNTokens(actionParts->rhs); // note here. expTokens is non owned

            //if (expTokens == nullptr) {
            //    return;
            //}

            Expressions::ReportInfo("\n***************** Action *********** rhs calc RPN: [");
            for (int i=0;i<expTokens->currentCount;i++) { // currentCount is set by GenerateRPNTokens and defines the current 'size'
                ExpressionToken& tok = expTokens->items[i];
                //if (tok->type == TokenType::Operand)
                    Expressions::ReportInfo(tok.ToString() + " ");
                //else
                //    ReportInfo(TokenTypeToString(tok->type ) + std::string(" "));
            }
            Expressions::ReportInfo("]\n\n");

            calcRpn = new CalcRPN(expTokens, 0, expTokens->currentCount);

            handlerOut = GetFunctionHandler(actionParts->op);
        }
        ActionStatement::~ActionStatement()
        {
            delete target;
            delete calcRpn;
        }
        
        HALOperationResult ActionStatement::Assign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            return actionItem->target->WriteSimple(val2write);
        }
        /*
        HALOperationResult ActionStatement::AddAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = val2write + readVal;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::SubtractAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal - val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::MultiplyAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal * val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::DivideAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal / val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::ModulusAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal & val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseOrAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal | val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal & val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseExOrAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal ^ val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseShiftRightAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal >> val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseShiftLeftAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal << val2write;
            return actionItem->target->WriteSimple(val2write);
        }

        ActionHandler ActionStatement::GetFunctionHandler(const char c) {
            if (c == '=') return &Assign_Handler;
            else if (c == '+') return &AddAndAssign_Handler;
            else if (c == '-') return &SubtractAndAssign_Handler;
            else if (c == '*') return &MultiplyAndAssign_Handler;
            else if (c == '/') return &DivideAndAssign_Handler;
            else if (c == '%') return &ModulusAssign_Handler;
            else if (c == '|') return &BitwiseOrAssign_Handler;
            else if (c == '&') return &BitwiseAndAssign_Handler;
            else if (c == '^') return &BitwiseExOrAssign_Handler;
            else if (c == '<') return &BitwiseShiftLeftAssign_Handler;
            else if (c == '>') return &BitwiseShiftRightAssign_Handler;
            else return nullptr; // should never happend
        }*/

        ActionHandler ActionStatement::GetFunctionHandler(const char c) {
            if (c == '=') return &Assign_Handler;
            else if (c == '+') return &CompoundAssign_Handler<OpAdd>;
            else if (c == '-') return &CompoundAssign_Handler<OpSub>;
            else if (c == '*') return &CompoundAssign_Handler<OpMul>;
            else if (c == '/') return &CompoundAssign_Handler<OpDiv>;
            else if (c == '%') return &CompoundAssign_Handler<OpMod>;
            else if (c == '|') return &CompoundAssign_Handler<OpBitOr>;
            else if (c == '&') return &CompoundAssign_Handler<OpBitAnd>;
            else if (c == '^') return &CompoundAssign_Handler<OpBitExOr>;
            else if (c == '<') return &CompoundAssign_Handler<OpBitLshift>;
            else if (c == '>') return &CompoundAssign_Handler<OpBitRshift>;
            else return nullptr; // should never happend
        }
    }
}