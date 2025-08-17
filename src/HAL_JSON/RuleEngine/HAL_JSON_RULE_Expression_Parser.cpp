
#include "HAL_JSON_RULE_Expression_Parser.h"

namespace HAL_JSON {
    namespace Rules {
        void Expressions::ReportError(const char* msg, const char* param) {
#ifdef _WIN32
            std::cout << "Error: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Error(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportWarning(const char* msg, const char* param) {
#ifdef _WIN32
            std::cout << "Warning: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Warn(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportInfo(std::string msg) {
#ifdef _WIN32
            std::cout << msg << std::endl;
#else
            //GlobalLogger.Info(F("Expr Rule Parse:"), msg);
#endif
        }

        const char* Expressions::SingleOperatorList = HAL_JSON_RULES_EXPRESSIONS_SINGLE_OPERATOR_LIST;
        const char* Expressions::DoubleOperatorList = HAL_JSON_RULES_EXPRESSIONS_DOUBLE_OPERATOR_LIST;

        bool Expressions::IsSingleOperator(char c) {
            const char* op = SingleOperatorList;
            while (*op != '\0') {
                if (*op == c) return true;
                ++op;
            }
            return false;
        }

        bool Expressions::IsDoubleOperator(const char* c) {
            if (c == nullptr) {
                ReportError("IsDoubleOperator c - was nullptr");
                return false;
            } else if (*c == '\n') {
                ReportError("IsDoubleOperator *c - was empty str");
                return false;
            } else if (*(c+1) == '\n') {
                ReportError("IsDoubleOperator *(c+1) - was empty str");
                return false;
            }
            //if (c == nullptr /*|| *c != '\0' */|| *(c + 1) == '\0') return false; // safety

            const char* op = DoubleOperatorList;
            while (*op != '\0') {
                if ((*op == *c) && (*(op+1) == *(c+1))) return true;
                op+=2;
            }
            return false;
        }

        bool Expressions::IsValidOperandChar(char c) {
            return 
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || 
                c == ':' || 
                c == '.' || 
                c == ',' || 
                c == '#';
        }

        bool Expressions::CountOperatorsAndOperands(Tokens& tokens, int& operatorCount, int& operandCount, int& leftParenthesisCount, ExpressionContext exprContext) {
            operatorCount = 0;
            operandCount = 0;
            leftParenthesisCount  = 0;
            // early checks
            if (tokens.count == 0) {
                ReportError("expr. is empty");
                return false;
            }
            /*if (strlen(expr) == 0) {
                ReportError("expr. is empty");
                return false;
            }*/

            const char* firstTokenStart = nullptr;
            if (tokens.firstTokenStartOffset != nullptr)
                firstTokenStart = tokens.firstTokenStartOffset;
            else
                firstTokenStart = tokens.items[0].start;

            if(IsDoubleOperator(firstTokenStart) && exprContext == ExpressionContext::IfCondition) { // this only checks the two first characters in the Expression
                ReportError("expr. cannot start with a operator");
                return false;
            }
            if(IsSingleOperator(*firstTokenStart)) {
                ReportError("expr. cannot start with a operator");
                return false;
            }

            int rightParenthesisCount = 0;

            bool inOperand = false;

            for (int cti=0;cti<tokens.count;cti++) { // cti = currTokenIndex
                Token& token = tokens.items[cti];
                const char* effectiveStart = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    effectiveStart  = tokens.firstTokenStartOffset;
                    //std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }
                for (const char* p = effectiveStart; p < token.end; p++) {
                    if (IsDoubleOperator(p) && exprContext == ExpressionContext::IfCondition) {
                        p++;
                        operatorCount++;
                        inOperand = false;
                    }
                    else if (IsSingleOperator(*p)) {
                        operatorCount++;
                        inOperand = false;
                    } else if (*p == '(') {
                        leftParenthesisCount++;
                        inOperand = false;
                    } else if (*p == ')') {
                        rightParenthesisCount++;
                        inOperand = false;
                    } else if (!inOperand) {
                        operandCount++;
                        inOperand = true;
                    }
                }
            }
            bool anyError = false;
            if (operatorCount >= operandCount) {
                ReportError("double operator(s) detected");
                anyError = true;
            } else if (operatorCount != operandCount - 1) {
                ReportError("operator(s) missing before/after parenthesis");
                anyError = true;
            }
            if (leftParenthesisCount != rightParenthesisCount) {
                ReportError("mismatch parenthesis detected");
                anyError = true;
            }

            return anyError == false;
        }
        
        

        bool Expressions::OperandIsVariable(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (isdigit(*p) == false) return true;
                p++;
            }
            return false;
        }

        const char* Expressions::ValidOperandVariableName(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (IsValidOperandChar(*p) == false) {
                    return p;
                }
                p++;
            }
            return nullptr;
        }

        bool Expressions::ValidateExpression(Tokens& tokens, ExpressionContext exprContext) {
            int operatorCount, operandCount, leftParenthesisCount;
            bool anyError = false;

            anyError = !CountOperatorsAndOperands(tokens, operatorCount, operandCount, leftParenthesisCount, exprContext);

            /*ReportInfo("operatorCount:" + std::to_string(operatorCount) +
                    ", operandCount:" + std::to_string(operandCount) +
                    ", leftParenthesisCount:" + std::to_string(leftParenthesisCount));
*/
            if (anyError) return false;

            int operandIndex = 0;
            bool inOperand = false;
            const char* p = nullptr;
            const char* operandStart = nullptr;
            const char* operandEnd = nullptr;

            for (int cti = 0; cti < tokens.count; ++cti) {
                Token& token = tokens.items[cti];
                const char* effectiveStart  = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    effectiveStart  = tokens.firstTokenStartOffset;
                    //std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }
                for (p = effectiveStart ; p < token.end; ++p) {
                    if (IsDoubleOperator(p)&& exprContext == ExpressionContext::IfCondition) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-effectiveStart);
                            ValidateOperand(operand, anyError);
                            ++operandIndex;
                            inOperand = false;
                        }
                        ++p; // Skip second char of double op
                    } else if (IsSingleOperator(*p) || *p == '(' || *p == ')' || *p == HAL_JSON_RULES_EXPRESSIONS_MULTILINE_KEYWORD[0]) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-effectiveStart);
                            ValidateOperand(operand, anyError);
                            ++operandIndex;
                            inOperand = false;
                        }
                    } else if (!inOperand) {
                        operandStart = p;
                        inOperand = true;
                    }
                }
                if (inOperand) { // this was outside of the last loop
                    operandEnd = p;
                    Token operand(operandStart, operandEnd);
                    operand.line = token.line;
                    operand.column = token.column + (operandStart-effectiveStart);
                    ValidateOperand(operand, anyError);
                    ++operandIndex;
                    inOperand = false;
                }
            }
            return !anyError;
        }

        void Expressions::ValidateOperand(const Token& operandToken, bool& anyError, ValidateOperandMode mode) {
            //bool operandIsVariable = OperandIsVariable(operandToken);
#ifdef HAL_JSON_RULES_EXPRESSIONS_PARSER_SHOW_DEBUG
            std::string msg;
            if (OperandIsVariable(operandToken)) {
                msg = "Variable: Name= ";
                ZeroCopyString funcName = operandToken;
                ZeroCopyString base = funcName.SplitOffHead('#');
                msg += base.ToString();
                if (funcName.Length() != 0) {
                    msg += ", funcName= ";
                    msg += funcName.ToString();
                }
            } else {
                msg = "Const Value: ";
                msg += operandToken.ToString();
            }
            ReportTokenInfo(operandToken, msg.c_str());
#endif
            // return early if not a operand variable
            if (OperandIsVariable(operandToken) == false)
                return;

            const char* currChar = ValidOperandVariableName(operandToken);
            if (currChar /*&& *currChar != '\0'*/) {
                std::string msg = "found invalid character <" + std::to_string(*currChar) +
                                "> in operand: " + operandToken.ToString();
                ReportTokenWarning(operandToken, msg.c_str());
                // continue validation but note the error
            }

            ZeroCopyString varOperand = operandToken;
            ZeroCopyString funcName = varOperand;
            varOperand = funcName.SplitOffHead('#');

            if (varOperand.Length() > HAL_UID::Size) {
                std::string msg = varOperand.ToString() + " length > " + std::to_string(HAL_UID::Size);
                
                ReportTokenError(operandToken, "Operand name too long: ", msg.c_str());
                anyError = true;
            }

            UIDPath path(varOperand);
            Device* device = Manager::findDevice(path);
            if (!device) {
                std::string deviceName = varOperand.ToString();
                ReportTokenError(operandToken, "could not find the device: ", deviceName.c_str());
                anyError = true;
                return;
            }
            if (mode == ValidateOperandMode::Read || mode == ValidateOperandMode::ReadWrite) {
                HALOperationResult readResult = HALOperationResult::UnsupportedOperation;
                if (funcName.Length() != 0) {
                    HALValue halValue;
                    HALReadValueByCmd readValueByCmd(halValue, funcName);
                    readResult = device->read(readValueByCmd);
                    if (readResult != HALOperationResult::Success) {
                        anyError = true;
                        if (readResult == HALOperationResult::UnsupportedCommand) {
                            std::string funcNameStr = ": " + funcName.ToString();
                            ReportTokenError(operandToken, ToString(readResult), funcNameStr.c_str());
                        } else {
                            ReportTokenError(operandToken, ToString(readResult), ": read");
                        }
                        
                    }
                } else {
                    HALValue halValue;
                    readResult = device->read(halValue);
                    if (readResult != HALOperationResult::Success) {
                        ReportTokenError(operandToken, ToString(readResult), ": read");
                        anyError = true;
                    }
                }
            }
            if (mode == ValidateOperandMode::Write || mode == ValidateOperandMode::ReadWrite) {
                HALOperationResult writeResult = HALOperationResult::UnsupportedOperation;
                if (funcName.Length() != 0) {
                    HALValue halValue;
                    HALWriteValueByCmd writeValueByCmd(halValue, funcName);
                    writeResult = device->write(writeValueByCmd);
                    if (writeResult != HALOperationResult::Success) {
                        anyError = true;
                        if (writeResult == HALOperationResult::UnsupportedCommand) {
                            std::string funcNameStr = ": " + funcName.ToString();
                            ReportTokenError(operandToken, ToString(writeResult), funcNameStr.c_str());
                        } else {
                            ReportTokenError(operandToken, ToString(writeResult), ": write");
                        }
                        
                    }
                } else {
                    HALValue halValue;
                    writeResult = device->write(halValue);
                    if (writeResult != HALOperationResult::Success) {
                        ReportTokenError(operandToken, ToString(writeResult), ": write");
                        anyError = true;
                    }
                }
            }
        }

        LogicRPN Expressions::BuildRPN(const Tokens& tokens) {
            LogicRPN result;

            std::vector<ZeroCopyString> current;        // collects tokens for one calc expression
            std::stack<ZeroCopyString> logicOpStack;    // &&, ||

            auto flushOperand = [&]() {
                if (!current.empty()) {
                    result.operands.push_back(ToCalcRPN(current));
                    current.clear();
                }
            };

            for (int cti = 0; cti < tokens.count; ++cti) {
                const Token& token = tokens.items[cti];
                ZeroCopyString t = token;  // implicit slice

                if (IsLogicOperator(t)) {
                    // finish previous calc expression
                    flushOperand();

                    // shunting-yard at logic level
                    while (!logicOpStack.empty() &&
                        LogicPrecedence(logicOpStack.top()) >= LogicPrecedence(t)) {
                        result.ops.push_back(logicOpStack.top());
                        logicOpStack.pop();
                    }
                    logicOpStack.push(t);

                } else {
                    // belongs to current calc expression
                    current.push_back(t);
                }
            }

            flushOperand();

            while (!logicOpStack.empty()) {
                result.ops.push_back(logicOpStack.top());
                logicOpStack.pop();
            }

            return result;
        }

        CalcRPN Expressions::ToCalcRPN(const std::vector<ZeroCopyString>& tokens) {
            CalcRPN rpn;
            std::stack<ZeroCopyString> opstack;

            for (auto& t : tokens) {
                if (IsCalcOperator(t)) {
                    while (!opstack.empty() && IsCalcOperator(opstack.top()) &&
                        CalcPrecedence(opstack.top()) >= CalcPrecedence(t)) {
                        rpn.tokens.push_back(opstack.top());
                        opstack.pop();
                    }
                    opstack.push(t);
                } else if (t == "(") {
                    opstack.push(t);
                } else if (t == ")") {
                    while (!opstack.empty() && opstack.top() != "(") {
                        rpn.tokens.push_back(opstack.top());
                        opstack.pop();
                    }
                    if (!opstack.empty()) opstack.pop(); // discard "("
                } else {
                    rpn.tokens.push_back(t);
                }
            }
            while (!opstack.empty()) {
                rpn.tokens.push_back(opstack.top());
                opstack.pop();
            }
            return rpn;
        }

        // Recursive print
        void Expressions::printLogicRPNNode(const LogicRPNNode& node) {
            if (node.op.IsEmpty()) {
                // Leaf node → print calc RPN
                std::cout << "[";
                for (size_t i = 0; i < node.calcRPN.size(); ++i) {
                    std::cout << node.calcRPN[i].ToString().c_str();
                    if (i + 1 < node.calcRPN.size()) std::cout << " ";
                }
                std::cout << "]";
            } else {
                // Operator node → print children first, then operator
                std::cout << "[";
                for (size_t i = 0; i < node.children.size(); ++i) {
                    printLogicRPNNode(node.children[i]);
                    if (i + 1 < node.children.size()) std::cout << " ";
                }
                std::cout << " " << node.op.ToString() << "]";
            }
        }

        LogicRPNNode Expressions::buildNestedLogicRPN(const Tokens& tokens) {
            std::stack<ZeroCopyString> opStack;          // logic operators &&, ||
            std::stack<LogicRPNNode> outStack;           // nested RPN nodes
            std::vector<ZeroCopyString> calcBuffer;      // temporary calc RPN

            auto flushCalcBuffer = [&]() {
                if (!calcBuffer.empty()) {
                    LogicRPNNode leaf;
                    leaf.calcRPN = calcBuffer;
                    outStack.push(leaf);
                    calcBuffer.clear();
                }
            };

            auto applyOperator = [&]() {
                ZeroCopyString op = opStack.top(); opStack.pop();
                LogicRPNNode right = outStack.top(); outStack.pop();
                LogicRPNNode left  = outStack.top(); outStack.pop();
                LogicRPNNode parent;
                parent.op = op;
                parent.children = {left, right};
                outStack.push(parent);
            };


            for (int cti = 0; cti < tokens.count; ++cti) {
                const Token& tok = tokens.items[cti];
                if (IsCalcOperator(tok)) {
                    // math/comparison → part of calcRPN
                    calcBuffer.push_back(tok);
                } else if (IsLogicOperator(tok)) {
                    // flush previous calcRPN as leaf
                    flushCalcBuffer();

                    while (!opStack.empty() && LogicPrecedence(opStack.top()) >= LogicPrecedence(tok)) {
                        applyOperator();
                        /*ZeroCopyString op = opStack.top(); opStack.pop();
                        LogicRPNNode right = outStack.top(); outStack.pop();
                        LogicRPNNode left  = outStack.top(); outStack.pop();
                        LogicRPNNode parent;
                        parent.op = op;
                        parent.children = {left, right};
                        outStack.push(parent);*/
                    }

                    opStack.push(tok);
                } else if (tok.Equals("(")) {
                    opStack.push(tok);
                } else if (tok.Equals(")")) {
                    flushCalcBuffer();
                    while (!opStack.empty() && opStack.top() != "(") {
                        applyOperator();
                        /*ZeroCopyString op = opStack.top(); opStack.pop();
                        LogicRPNNode right = outStack.top(); outStack.pop();
                        LogicRPNNode left  = outStack.top(); outStack.pop();
                        LogicRPNNode parent;
                        parent.op = op;
                        parent.children = {left, right};
                        outStack.push(parent);*/
                    }
                    opStack.pop(); // remove '('
                } else {
                    // Variable/constant → part of calcRPN
                    calcBuffer.push_back(tok);
                }
            }

            flushCalcBuffer();

            while (!opStack.empty()) {
                applyOperator();
                /*ZeroCopyString op = opStack.top(); opStack.pop();
                LogicRPNNode right = outStack.top(); outStack.pop();
                LogicRPNNode left  = outStack.top(); outStack.pop();
                LogicRPNNode parent;
                parent.op = op;
                parent.children = {left, right};
                outStack.push(parent);*/
            }

            return outStack.top();
        }

/*
LogicRPNNode Expressions::buildNestedLogicRPN(const Tokens& tokens) {
    std::stack<ZeroCopyString> opStack;          // logic operators &&, ||
    std::stack<LogicRPNNode> outStack;           // nested RPN nodes
    std::vector<ZeroCopyString> calcBuffer;      // temporary calc RPN

    auto flushCalcBuffer = [&]() {
        if (!calcBuffer.empty()) {
            LogicRPNNode leaf;
            leaf.calcRPN = calcBuffer;
            outStack.push(leaf);
            calcBuffer.clear();
        }
    };

    auto applyOperator = [&](const ZeroCopyString& op) {
        // Pop at least two nodes
        LogicRPNNode right = outStack.top(); outStack.pop();
        LogicRPNNode left  = outStack.top(); outStack.pop();

        LogicRPNNode parent;
        parent.op = op;

        // Flatten if left node has the same operator
        if (left.op == op) {
            parent.children = left.children;   // take left's children
            parent.children.push_back(right);  // append right
        } else {
            parent.children = {left, right};
        }

        outStack.push(parent);
    };


    for (int cti = 0; cti < tokens.count; ++cti) {
        const Token& tok = tokens.items[cti];

        if (IsCalcOperator(tok)) {
            // math/comparison → part of calcRPN
            calcBuffer.push_back(tok);
        } else if (IsLogicOperator(tok)) {
            flushCalcBuffer();

            while (!opStack.empty() && LogicPrecedence(opStack.top()) >= LogicPrecedence(tok)) {
                ZeroCopyString op = opStack.top(); opStack.pop();
                applyOperator(op);
            }

            opStack.push(tok);
        } else if (tok.Equals("(")) {
            opStack.push(tok);
        } else if (tok.Equals(")")) {
            flushCalcBuffer();
            while (!opStack.empty() && opStack.top() != "(") {
                ZeroCopyString op = opStack.top(); opStack.pop();
                applyOperator(op);
            }
            opStack.pop(); // remove '('
        } else {
            // Variable/constant → part of calcRPN
            calcBuffer.push_back(tok);
        }
    }

    flushCalcBuffer();

    while (!opStack.empty()) {
        ZeroCopyString op = opStack.top(); opStack.pop();
        applyOperator(op);
    }

    return outStack.top();
}*/

    }
}