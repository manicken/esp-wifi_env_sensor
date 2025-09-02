

#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"

namespace HAL_JSON {
    namespace Rules {
        RPNStack<HALValue> halValueStack;

        CalcRPNToken::CalcRPNToken()
        {

        }
        CalcRPNToken::~CalcRPNToken()
        {
            if (deleter) deleter(context);
        }
        CalcRPN::CalcRPN(ExpressionTokens& tokens, int startIndex, int endIndex) {
            calcRPNcount = endIndex - startIndex;
            calcRPN = new CalcRPNToken[calcRPNcount];
            int calcRPNindex = 0;
            ExpressionToken* tokenItems = tokens.items;
            for (int i=startIndex;i<endIndex;i++, calcRPNindex++) {
                ExpressionToken& token = tokenItems[i];
                if (token.type == ExpTokenType::VarOperand) {
                    calcRPN[calcRPNindex].handler = &CalcRPNToken::GetAndPushVariableValue_Handler;
                    
                    const char* uidPath = nullptr;
                    const char* funcName = nullptr;
                    // TODO extract above from token
                    
                    CachedDeviceAccess* cda = new CachedDeviceAccess(uidPath, funcName);
                    calcRPN[calcRPNindex].context = cda;
                    calcRPN[calcRPNindex].deleter = DeleteAs<CachedDeviceAccess>;
                } else if (token.type == ExpTokenType::ConstValOperand) {
                    calcRPN[calcRPNindex].handler = &CalcRPNToken::GetAndPushConstValue_Handler;
                    NumberResult constNumber = token.ConvertStringToNumber();
                    HALValue* value = new HALValue();
                    if (constNumber.type == NumberType::FLOAT)
                        value->set(constNumber.f32);
                    else if (constNumber.type == NumberType::INT32)
                        value->set(constNumber.i32);
                    else if (constNumber.type == NumberType::UINT32)
                        value->set(constNumber.u32);
                    else { // should never happend
                        std::string msg = token.ToString();
                        GlobalLogger.Error(F("fail of converting constant default is set to one"), msg.c_str()); // remainder the string is copied internally here
                        value->set(1); // default one so any divide by zero would not happend
                    }
                    calcRPN[calcRPNindex].context = value;
                    calcRPN[calcRPNindex].deleter = DeleteAs<HALValue>;
                } else {
                    calcRPN[calcRPNindex].handler = CalcRPNToken::GetRPN_OperatorFunction(token.type);
                    calcRPN[calcRPNindex].deleter = nullptr; // not used here
                }
            }
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




        HALOperationResult CalcRPNToken::Calc_Addition_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_Subtract_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_Multiply_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_Divide_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_Modulus_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_BitwiseAnd_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_BitwiseOr_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_BitwiseExOr_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_BitwiseLeftShift_Operation_Handler(void* context) {
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
        HALOperationResult CalcRPNToken::Calc_BitwiseRightShift_Operation_Handler(void* context) {
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

        HALOperationResult CalcRPNToken::Compare_NotEqualsTo_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a != b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_EqualsTo_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a == b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_LessThan_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a < b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_GreaterThan_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a > b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_LessThanOrEqual_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a <= b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_GreaterThanOrEqual_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_RULES_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a >= b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }

        RPNHandler CalcRPNToken::GetRPN_OperatorFunction(ExpTokenType type) {
            switch (type) {
                case ExpTokenType::CompareEqualsTo: return &Compare_EqualsTo_Operation_Handler;
                case ExpTokenType::CompareNotEqualsTo: return &Compare_NotEqualsTo_Operation_Handler;
                case ExpTokenType::CompareLessThan: return &Compare_LessThan_Operation_Handler;
                case ExpTokenType::CompareGreaterThan: return &Compare_GreaterThan_Operation_Handler;
                case ExpTokenType::CompareLessThanOrEqual: return &Compare_LessThanOrEqual_Operation_Handler;
                case ExpTokenType::CompareGreaterThanOrEqual: return &Compare_GreaterThanOrEqual_Operation_Handler;
                case ExpTokenType::CalcPlus: return &Calc_Addition_Operation_Handler;
                case ExpTokenType::CalcMinus: return &Calc_Subtract_Operation_Handler;
                case ExpTokenType::CalcMultiply: return &Calc_Multiply_Operation_Handler;
                case ExpTokenType::CalcDivide: return &Calc_Divide_Operation_Handler;
                case ExpTokenType::CalcModulus: return &Calc_Modulus_Operation_Handler;
                case ExpTokenType::CalcBitwiseAnd: return &Calc_BitwiseAnd_Operation_Handler;
                case ExpTokenType::CalcBitwiseOr: return &Calc_BitwiseOr_Operation_Handler;
                case ExpTokenType::CalcBitwiseExOr: return &Calc_BitwiseExOr_Operation_Handler;
                case ExpTokenType::CalcBitwiseLeftShift: return &Calc_BitwiseLeftShift_Operation_Handler;
                case ExpTokenType::CalcBitwiseRightShift: return &Calc_BitwiseRightShift_Operation_Handler;
                default: return nullptr; // handled by caller
            }
        }

    }
}