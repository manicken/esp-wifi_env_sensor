
#include "HAL_JSON_RULE_Engine_Structures.h"

#define HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS

namespace HAL_JSON {
    namespace Rule {
        namespace Structures {
            RPNStack::RPNStack(int size) : size(size), items(new HALValue[size]), sp(0) { }
            RPNStack::~RPNStack() {
                delete[] items;
            }


            bool RPNStack::GetFinalResult(HALValue& value) {
                if (sp != 1) { // make sure that there is only one item left
                    // log error
                    return false;
                }
                value = items[--sp];
                return true;
            }

            OpBlock::OpBlock() : type(OpBlockType::NotSet) 
            {
                unset = nullptr;
            }

            OpBlock::~OpBlock()
            {
                if (type == OpBlockType::If) {
                    delete ifData;
                } else if (type == OpBlockType::Exec) {
                    delete execData;
                } // else its unset so nothing needs freeing
            }

            HALOperationResult RPNcalc_Item::GetAndPushVariableValue_Handler(void* context, RPNStack& stack) {
                RPNcalc_Item* item = static_cast<RPNcalc_Item*>(context);
                HALValue value;
                if (item->cDevice->valueDirectAccessPtr != nullptr) {
                    value = *item->cDevice->valueDirectAccessPtr;
                }
                else if (item->cDevice->readToHalValueFunc != nullptr) {
                    Device* device = item->cDevice->GetDevice();
                    HALOperationResult result = item->cDevice->readToHalValueFunc(device, value);
                    if (result != HALOperationResult::Success) return result;
                } else {
                    Device* device = item->cDevice->GetDevice();
                    HALOperationResult result = device->read(value);
                    if (result != HALOperationResult::Success) return result;
                }
                    
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (stack.sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                stack.items[stack.sp++] = value;
                return HALOperationResult::Success;
            }

            HALOperationResult RPNcalc_Item::GetAndPushConstValue_Handler(void* context, RPNStack& stack) {
                RPNcalc_Item* item = static_cast<RPNcalc_Item*>(context);
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (stack.sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                stack.items[stack.sp++] = *item->constValue;
                return HALOperationResult::Success;
            }


            HALOperationResult RPNcalc_Item::Add_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a + b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::Subtract_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a - b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::Multiply_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a * b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::Divide_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                if (b.asInt() == 0) {
                    // Log to GlobalLogger about divide by zero
                    return HALOperationResult::DivideByZero;
                }
                HALValue a = *--itemPtr;
                *itemPtr++ = (a / b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::Modulus_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                if (b.asInt() == 0) {
                    // Log to GlobalLogger about divide by zero
                    return HALOperationResult::DivideByZero;
                }
                HALValue a = *--itemPtr;
                *itemPtr++ = (a % b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::BitwiseAnd_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a & b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::BitwiseOr_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a | b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::BitwiseExOr_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a ^ b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::BitwiseLeftShift_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a << b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            HALOperationResult RPNcalc_Item::BitwiseRightShift_Operation_Handler(void* context, RPNStack& stack) {
                int sp = stack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
                if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
                if (sp >= stack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
                HALValue* itemPtr = &stack.items[sp];
                HALValue b = *--itemPtr;
                HALValue a = *--itemPtr;
                *itemPtr++ = (a >> b);
                stack.sp = sp - 1;
                return HALOperationResult::Success;
            }
            bool RPNCondition::NotEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
                return lhs != rhs;
            }
            bool RPNCondition::Equals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
                return lhs == rhs;
            }
            bool RPNCondition::LessThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
                return lhs < rhs;
            }
            bool RPNCondition::LargerThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
                return lhs > rhs;
            }
            bool RPNCondition::LessThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
                return lhs <= rhs;
            }
            bool RPNCondition::LargerThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
                return lhs >= rhs;
            }
        }
    }
}