
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"
#include "HAL_JSON_Manager.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Operations.h"

#include <vector> // have this for easy prototyping
#include <variant>

namespace HAL_JSON {

    enum class ConditionType {
        Equals,
        NotEquals,
        LessThan,
        GreaterThan,
        LessOrEqual,
        GreaterOrEqual
    };

    enum class ActionType {
        SetLocalVariable,
        //Increment,
        HalWrite,
        //Toggle,
        // Extend later
    };
    enum class OperandType {
        Path,
        LocalValue,
        Value
    };

    struct Operand {
        OperandType type;
        union {
            UIDPath* path;
            HALValue value;
            uint32_t localValueIndex;
        };
    };

    struct Condition {
        Operand left;
        Operand right;
        ConditionType type;
    };

    struct ActionTarget {
        OperandType type;
        union {
            UIDPath* path;
            uint32_t localValueIndex;
        };
    };

    struct Action {
        ActionTarget target;
        ActionType type;
        HALValue value;
    };

    struct Rule {
        std::vector<Condition> conditions;
        std::vector<Action> actions;
        bool allConditionsMustPass = true;  // AND/OR behavior
    };

    inline constexpr int RULE_MGR_MAX_VARIABLES = 32;

    class RuleManager {
        //inline constexpr int MAX_VARIABLES = 16;
    private:
        
        static CharArray::StringView tempVarNames[RULE_MGR_MAX_VARIABLES];
        static std::vector<Rule> rules;
        //static Rule rules[16]; // use when im sure that everything else works
        static HALValue variables[RULE_MGR_MAX_VARIABLES];
        static HALValue resolveOperand(const Operand& op);
        static bool evaluateCondition(const Condition& cond);
        static void executeAction(const Action& action);

        static bool decodeRuleLineType(const char* lineStr, const char* str);
        /** a note here if length is zero then the length of ruleStr is used */
        static bool validateRuleLine(const char* ruleStr, size_t length = 0);
    public:
        static void evaluateRules();
        static bool validateRuleSet(const char* ruleSet);
        
        static void addRule(const char* ruleStr);

    };
}