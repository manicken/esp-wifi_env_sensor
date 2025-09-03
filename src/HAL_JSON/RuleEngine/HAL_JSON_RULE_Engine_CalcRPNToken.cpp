

#include "HAL_JSON_RULE_Engine_CalcRPNToken.h"

namespace HAL_JSON {
    namespace Rules {
        RPNStack<HALValue> halValueStack;

        CalcRPNToken::CalcRPNToken() { }
        CalcRPNToken::~CalcRPNToken()
        {
            if (deleter) deleter(context);
        }
        CalcRPN::CalcRPN(ExpressionTokens* tokens, int startIndex, int endIndex) {
            count = endIndex - startIndex;
            items = new CalcRPNToken[count];
            int calcRPNindex = 0;
            ExpressionToken* tokenItems = tokens->items;
            for (int i=startIndex;i<endIndex;i++, calcRPNindex++) {
                ExpressionToken& expToken = tokenItems[i];
                CalcRPNToken& calcRPNtoken = items[calcRPNindex];
                if (expToken.type == ExpTokenType::VarOperand)
                {
                    ZeroCopyString varOperand;
                    ZeroCopyString funcName = expToken;
                    varOperand = funcName.SplitOffHead('#');
                    CachedDeviceAccess* cda = new CachedDeviceAccess(varOperand, funcName);

                    calcRPNtoken.context = cda;
                    calcRPNtoken.handler = &CalcRPNToken::GetAndPushVariableValue_Handler;
                    calcRPNtoken.deleter = DeleteAs<CachedDeviceAccess>;
                }
                else if (expToken.type == ExpTokenType::ConstValOperand)
                {
                    NumberResult constNumber = expToken.ConvertStringToNumber();
                    HALValue* value = new HALValue();
                    if (constNumber.type == NumberType::FLOAT)
                        value->set(constNumber.f32);
                    else if (constNumber.type == NumberType::INT32)
                        value->set(constNumber.i32);
                    else if (constNumber.type == NumberType::UINT32)
                        value->set(constNumber.u32);
                    else { // should never happend
                        std::string msg = expToken.ToString();
                        GlobalLogger.Error(F("fail of converting constant default is set to one"), msg.c_str()); // remainder the string is copied internally here
                        value->set(1); // default one so any divide by zero would not happend
                    }

                    calcRPNtoken.context = value;
                    calcRPNtoken.handler = &CalcRPNToken::GetAndPushConstValue_Handler;
                    calcRPNtoken.deleter = DeleteAs<HALValue>;
                }
                else
                {
                    calcRPNtoken.context = nullptr; // not used here
                    calcRPNtoken.handler = CalcRPNToken::GetRPN_OperatorFunction(expToken.type);
                    calcRPNtoken.deleter = nullptr; // not used here
                }
            }
        }
        HALOperationResult CalcRPN::DoCalc() {
            //CalcRPN* calcRPN = static_cast<CalcRPN*>(context);
            HALOperationResult res;
            int calcRPNcount = count;
            CalcRPNToken* calcItems = items;
            for (int i=0;i<calcRPNcount;i++) {
                res = calcItems[i].handler(calcItems[i].context);
                if (res != HALOperationResult::Success)
                    return res;
            }
            return res;
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