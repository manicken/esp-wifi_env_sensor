

#include "HAL_JSON_RULE_Engine.h"

namespace HAL_JSON {

    std::vector<Rule> RuleEngine::rules;

    HALValue RuleEngine::resolveOperand(const Operand& op) {
        if (op.type == OperandType::Path) {
            HALValue value;
            HALReadRequest req(*op.path, value);
            Manager::read(req);
            return value;
        }/* else if (op.type == OperandType::LocalValue) {
            return variables[op.localValueIndex];
        }*/ else if (op.type == OperandType::Value) {
            return op.value;
        }
        return HALValue(); // default
    }

    bool RuleEngine::evaluateCondition(const Condition& cond) {
        HALValue left = resolveOperand(cond.left);
        HALValue right = resolveOperand(cond.right);
        switch (cond.type) {
            case ConditionType::Equals: return left == right;
            case ConditionType::NotEquals: return left != right;
            case ConditionType::LessThan: return left < right;
            case ConditionType::GreaterThan: return left > right;
            case ConditionType::LessOrEqual: return left <= right;
            case ConditionType::GreaterOrEqual: return left >= right;
        }
        return false;
    }
    void RuleEngine::executeAction(const Action& action) {
        HALValue value = action.value;
        if (action.type == ActionType::HalWrite) {
            HALWriteRequest req = HALWriteRequest(*action.path, value);
            Manager::write(req);
        }
        else if (action.type == ActionType::HalRead) {
            HALReadRequest req = HALReadRequest(*action.path, value);
            Manager::read(req);
        }
    }
    void RuleEngine::evaluateRules() {
        for (const auto& rule : rules) {
            if (rule.conditions.empty())
                continue;

            bool result = rule.allConditionsMustPass;
            for (const auto& cond : rule.conditions) {
                bool conditionResult = evaluateCondition(cond);
                if (rule.allConditionsMustPass)
                    result &= conditionResult;
                else
                    result |= conditionResult;
            }

            if (result) {
                for (const auto& action : rule.actions)
                    executeAction(action);
            }
        }
    }
    bool RuleEngine::validateRuleSet(const char* ruleSet) {
        uint32_t newLineIndicies_Count = 0;
        const char** newLinePointers = CharArray::getPointers(ruleSet, '\n', newLineIndicies_Count);
        if (newLineIndicies_Count == 0) {
             return validateRuleLine(ruleSet);
        }

        const char* currLine = ruleSet;
        bool allValid = true;

        for (uint32_t i = 0; i < newLineIndicies_Count; i++) {
            size_t len = newLinePointers[i] - currLine;
            if (currLine && !validateRuleLine(currLine, len)) {
                allValid = false;
                break;
            }
            currLine = newLinePointers[i] + 1; // Move past '\n'
        }

        // Handle last line if file does not end with '\n'
        if (*currLine != '\0') {
            if (currLine && !validateRuleLine(currLine)) {
                allValid = false;
            }
        }

        delete[] newLinePointers;
        return allValid;
    }

    bool RuleEngine::decodeRuleLineType(const char* lineStr, const char* str) {
        const char* found = strstr(lineStr, str);
        if (!found) {
            // str not found at all
            return false;
        }
        size_t strLen = strlen(str);
        if ((found == lineStr && isspace(found[strLen])) || (found > lineStr && isspace(found[-1]))) {
            return true;// OK: starts with str or is preceded by space
        } else {
            return false;
        }
    }

    bool RuleEngine::validateRuleLine(const char* ruleStr, size_t length) {
        
        if (length == 0) length = strlen(ruleStr);
        if (decodeRuleLineType(ruleStr, "on")){

        } else if (decodeRuleLineType(ruleStr, "var")) {

        } 
        return false;

    }

    void RuleEngine::addRule(const char* ruleStr) {
        
    }
}