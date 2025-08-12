
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

            typedef HALOperationResult (*OperatorFunc)(void* context, RPNStack& stack);
            typedef bool (*ConditionFunc)(const HALValue& lhs, const HALValue& rhs);
            typedef HALOperationResult (*OpBlockFunc)(void* context);

            // helpers
            template<typename T>
            void DeleteAs(void* ptr) {
                delete static_cast<T*>(ptr);
            }
            typedef void (*Deleter)(void* context);

            enum class RPNlogical_ItemTypes {
                Operand,
                Operator
            };

            struct RPNcalc_Item {
                HALOperationResult (*handler)(RPNcalc_Item* context, RPNStack& stack);

                union {
                    CachedDeviceAccess* cDevice;
                    HALValue* constValue;
                };

                RPNcalc_Item(RPNcalc_Item&) = delete;          // no copy constructor
                RPNcalc_Item& operator=(const RPNcalc_Item&) = delete; // no copy assignment
                RPNcalc_Item(RPNcalc_Item&& other) = delete;           // no move constructor
                RPNcalc_Item& operator=(RPNcalc_Item&& other) = delete; // no move assignment

                static HALOperationResult GetAndPushVariableValue_Handler(void* context, RPNStack& stack);
                static HALOperationResult GetAndPushConstValue_Handler(void* context, RPNStack& stack);

                static HALOperationResult Add_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult Subtract_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult Multiply_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult Divide_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult Modulus_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult BitwiseAnd_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult BitwiseOr_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult BitwiseExOr_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult BitwiseLeftShift_Operation_Handler(void* context, RPNStack& stack);
                static HALOperationResult BitwiseRightShift_Operation_Handler(void* context, RPNStack& stack);

                RPNcalc_Item();
                ~RPNcalc_Item();
            };

            struct RPNCondition {
                ConditionFunc handler;

                RPNcalc_Item* lhsItems;
                int lhsCount;
                RPNcalc_Item* rhsItems;
                int rhsCount;

                RPNCondition(RPNCondition&) = delete;          // no copy constructor
                RPNCondition& operator=(const RPNCondition&) = delete; // no copy assignment
                RPNCondition(RPNCondition&& other) = delete;           // no move constructor
                RPNCondition& operator=(RPNCondition&& other) = delete; // no move assignment

                static bool NotEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
                static bool Equals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
                static bool LessThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
                static bool LargerThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
                static bool LessThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);
                static bool LargerThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs);

                RPNCondition();
                ~RPNCondition();
            };

            struct RPNlogical_Item {
                OperatorFunc handler;

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

            struct IfBlock
            {
                RPNlogical_Item* conditionItems;
                int conditionItemsCount;
                OpBlock* opItems;
                int opItemsCount;

                IfBlock(IfBlock&) = delete;          // no copy constructor
                IfBlock& operator=(const IfBlock&) = delete; // no copy assignment
                IfBlock(IfBlock&& other) = delete;           // no move constructor
                IfBlock& operator=(IfBlock&& other) = delete; // no move assignment

                IfBlock();
                ~IfBlock();
            };

            struct ExecBlock
            {
                CachedDeviceAccess* target;
                RPNcalc_Item* sourceCalcItems;
                int sourceCalcItemsCount;

                ExecBlock(ExecBlock&) = delete;          // no copy constructor
                ExecBlock& operator=(const ExecBlock&) = delete; // no copy assignment
                ExecBlock(ExecBlock&& other) = delete;           // no move constructor
                ExecBlock& operator=(ExecBlock&& other) = delete; // no move assignment

                ExecBlock();
                ~ExecBlock();
            };

            struct OpBlock
            {
                void* context;
                OpBlockFunc handler;
                Deleter deleter;                
 
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