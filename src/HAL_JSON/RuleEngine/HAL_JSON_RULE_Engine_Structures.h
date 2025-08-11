
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {
    namespace Rule {
        namespace Structures {

            typedef void (*OperatorFunc)(const uint8_t **pc, int32_t *stack, int *sp);


            enum class OpBlockType : uint8_t
            {
                NotSet,
                If,
                Exec
            };

            enum class ConditionType {
                Equals,
                LessThan,
                LargerThan,
                LessThanOrEquals,
                LargerThanOrEquals
            };

            enum class RPNcalc_ItemTypes {
                ConstOperand,
                VarOperand,
                Operator
            };
            enum class RPNlogical_ItemTypes {
                Operand,
                Operator
            };

            struct RPNcalc_Item {
                RPNcalc_ItemTypes type;

                union {
                    CachedDeviceAccess cDevice;
                    HALValue constValue;
                    OperatorFunc op;
                };
            };

            struct RPNCondition {
                ConditionType type;
                RPNcalc_Item* lhsItems;
                int lhsCount;
                RPNcalc_Item* rhsItems;
                int rhsCount;
            };

            struct RPNlogical_Item {
                RPNlogical_ItemTypes type;
                union {
                    RPNCondition item;
                    OperatorFunc op;
                };
            };

            struct IfData
            {
                RPNlogical_Item* conditionItems;
                int conditionItemsCount;
                OpBlock* opItems;
                int opItemsCount;
            };

            struct ExecData
            {
                CachedDeviceAccess* target;
                RPNcalc_Item* sourceCalcItems;
                int sourceCalcItemsCount;
            };

            struct OpBlock
            {
                OpBlockType type;
                union
                {
                    const void* unset;
                    IfData ifData;
                    ExecData execData;
                };

                OpBlock();
                ~OpBlock();
                static OpBlock MakeIf(const void* cond, uint16_t firstChild, uint16_t count);
                static OpBlock MakeExec(const void* func, uint16_t a1, uint16_t a2);
            };
            
        }
    }
}