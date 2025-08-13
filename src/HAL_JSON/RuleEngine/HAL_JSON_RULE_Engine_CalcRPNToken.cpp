

#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"

namespace HAL_JSON {
    namespace Rule {
        RPNStack<HALValue> halValueStack;

        CalcRPNToken::CalcRPNToken()
        {

        }
        CalcRPNToken::~CalcRPNToken()
        {
            if (deleter) deleter(context);
        }

        HALOperationResult CalcRPNToken::GetAndPushVariableValue_Handler(void* context) {
            CachedDeviceAccess* item = static_cast<CachedDeviceAccess*>(context);
            HALValue value;
            if (item->valueDirectAccessPtr != nullptr) {
                value = *item->valueDirectAccessPtr;
            }
            else if (item->readToHalValueFunc != nullptr) {
                Device* device = item->GetDevice();
                HALOperationResult result = item->readToHalValueFunc(device, value);
                if (result != HALOperationResult::Success) return result;
            } else {
                Device* device = item->GetDevice();
                HALOperationResult result = device->read(value);
                if (result != HALOperationResult::Success) return result;
            }
                
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            halValueStack.items[halValueStack.sp++] = value;
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::GetAndPushConstValue_Handler(void* context) {
            HALValue* item = static_cast<HALValue*>(context);
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            halValueStack.items[halValueStack.sp++] = *item;
            return HALOperationResult::Success;
        }


        HALOperationResult CalcRPNToken::Add_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a + b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Subtract_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a - b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Multiply_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a * b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Divide_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            if (b.asInt() == 0) {
                // Log to GlobalLogger about divide by zero
                return HALOperationResult::DivideByZero;
            }
            HALValue a = *--itemPtr;
            *itemPtr++ = (a / b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Modulus_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            if (b.asInt() == 0) {
                // Log to GlobalLogger about divide by zero
                return HALOperationResult::DivideByZero;
            }
            HALValue a = *--itemPtr;
            *itemPtr++ = (a % b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::BitwiseAnd_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a & b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::BitwiseOr_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a | b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::BitwiseExOr_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a ^ b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::BitwiseLeftShift_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a << b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::BitwiseRightShift_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a >> b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
    }
}