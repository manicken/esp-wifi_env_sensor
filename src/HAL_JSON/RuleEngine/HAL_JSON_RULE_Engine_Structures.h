
#pragma once

#include <Arduino.h>
#include "../HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {
    namespace Rule {
        namespace Structures {

            class RPNStack {
            public:
                const int size;
                HALValue* items;
                int sp;
                
                RPNStack() = delete;
                RPNStack(int size);

                bool GetFinalResult(HALValue& value);

            private:
                RPNStack(RPNStack&) = delete;          // no copy constructor
                RPNStack& operator=(const RPNStack&) = delete; // no copy assignment
                RPNStack(RPNStack&& other) = delete;           // no move constructor
                RPNStack& operator=(RPNStack&& other) = delete; // no move assignment
            };

            typedef bool (*OperatorFunc)(void* context, RPNStack& stack);

            enum class OpBlockType {
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

            enum class RPNlogical_ItemTypes {
                Operand,
                Operator
            };

            struct RPNcalc_Item {
                OperatorFunc handler;

                union {
                    CachedDeviceAccess* cDevice;
                    HALValue* constValue;
                };

                RPNcalc_Item(RPNcalc_Item&) = delete;          // no copy constructor
                RPNcalc_Item& operator=(const RPNcalc_Item&) = delete; // no copy assignment
                RPNcalc_Item(RPNcalc_Item&& other) = delete;           // no move constructor
                RPNcalc_Item& operator=(RPNcalc_Item&& other) = delete; // no move assignment

                static HALOperationResult  GetAndPushVariableValue_Handler(void* context, RPNStack& stack);
                static HALOperationResult  GetAndPushConstValue_Handler(void* context, RPNStack& stack);

                static HALOperationResult  Add_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  Subtract_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  Multiply_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  Divide_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  Modulus_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  BitwiseAnd_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  BitwiseOr_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  BitwiseExOr_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  BitwiseLeftShift_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult  BitwiseRightShift_Operation_Handler(void* context, RPNStack& stack);

                RPNcalc_Item();
                ~RPNcalc_Item();
            };

            struct RPNCondition {
                ConditionType type;
                RPNcalc_Item* lhsItems;
                int lhsCount;
                RPNcalc_Item* rhsItems;
                int rhsCount;

                RPNCondition(RPNCondition&) = delete;          // no copy constructor
                RPNCondition& operator=(const RPNCondition&) = delete; // no copy assignment
                RPNCondition(RPNCondition&& other) = delete;           // no move constructor
                RPNCondition& operator=(RPNCondition&& other) = delete; // no move assignment

                RPNCondition();
                ~RPNCondition();
            };

            struct RPNlogical_Item {
                RPNlogical_ItemTypes type;
                union {
                    RPNCondition* item;
                    OperatorFunc op;
                };

                RPNlogical_Item(RPNlogical_Item&) = delete;          // no copy constructor
                RPNlogical_Item& operator=(const RPNlogical_Item&) = delete; // no copy assignment
                RPNlogical_Item(RPNlogical_Item&& other) = delete;           // no move constructor
                RPNlogical_Item& operator=(RPNlogical_Item&& other) = delete; // no move assignment

                RPNlogical_Item();
                ~RPNlogical_Item();
            };

            struct IfData
            {
                RPNlogical_Item* conditionItems;
                int conditionItemsCount;
                OpBlock* opItems;
                int opItemsCount;

                IfData(IfData&) = delete;          // no copy constructor
                IfData& operator=(const IfData&) = delete; // no copy assignment
                IfData(IfData&& other) = delete;           // no move constructor
                IfData& operator=(IfData&& other) = delete; // no move assignment

                IfData();
                ~IfData();
            };

            struct ExecData
            {
                CachedDeviceAccess* target;
                RPNcalc_Item* sourceCalcItems;
                int sourceCalcItemsCount;

                ExecData(ExecData&) = delete;          // no copy constructor
                ExecData& operator=(const ExecData&) = delete; // no copy assignment
                ExecData(ExecData&& other) = delete;           // no move constructor
                ExecData& operator=(ExecData&& other) = delete; // no move assignment

                ExecData();
                ~ExecData();
            };

            struct OpBlock
            {
                OpBlockType type;
                union
                {
                    const void* unset;
                    IfData* ifData;
                    ExecData* execData;
                };

                OpBlock(OpBlock&) = delete;          // no copy constructor
                OpBlock& operator=(const OpBlock&) = delete; // no copy assignment
                OpBlock(OpBlock&& other) = delete;           // no move constructor
                OpBlock& operator=(OpBlock&& other) = delete; // no move assignment

                OpBlock();
                ~OpBlock();
            };
            
        }
    }
}