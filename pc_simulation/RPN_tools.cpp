
#include "RPN_tools.h"

namespace HAL_JSON
{
    #define MAX_TOKENS 64

    enum Assoc { LEFT, RIGHT };

    struct OperatorInfo {
        const char* op;
        int precedence;
        Assoc assoc;
    };

    const char* operatorList = "+-*/|&^";

    bool IsOperator(char c) {
        const char* op = operatorList;
        while (*op != '\0') {
            if (*op == c) return true;
            ++op;
        }
        return false;
    }

    OperatorInfo operators[] = {
        { "+", 1, LEFT },
        { "-", 1, LEFT },
        { "*", 2, LEFT },
        { "/", 2, LEFT },
        { "(", 0, LEFT },
        { ")", 0, LEFT }
    };

    int GetPrecedence(const char* op) {
        for (size_t i = 0; i < sizeof(operators) / sizeof(operators[0]); i++) {
            if (strcmp(operators[i].op, op) == 0) {
                return operators[i].precedence;
            }
        }
        return -1; // unknown
    }

    Assoc GetAssociativity(const char* op) {
        for (size_t i = 0; i < sizeof(operators) / sizeof(operators[0]); i++) {
            if (strcmp(operators[i].op, op) == 0) {
                return operators[i].assoc;
            }
        }
        return LEFT;
    }

    bool IsOperator(const char* token) {
        return strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
            strcmp(token, "*") == 0 || strcmp(token, "/") == 0;
    }

    bool ToRPN(const char* tokens[], int tokenCount, const char* outputRPN[], int& outCount) {
        const char* stack[MAX_TOKENS];
        int stackTop = 0;
        outCount = 0;

        for (int i = 0; i < tokenCount; ++i) {
            const char* token = tokens[i];

            if (IsOperator(token)) {
                while (stackTop > 0) {
                    const char* top = stack[stackTop - 1];
                    if (IsOperator(top) &&
                        ((GetAssociativity(token) == LEFT && GetPrecedence(token) <= GetPrecedence(top)) ||
                        (GetAssociativity(token) == RIGHT && GetPrecedence(token) < GetPrecedence(top)))) {
                        outputRPN[outCount++] = stack[--stackTop];
                    } else {
                        break;
                    }
                }
                stack[stackTop++] = token;
            }
            else if (strcmp(token, "(") == 0) {
                stack[stackTop++] = token;
            }
            else if (strcmp(token, ")") == 0) {
                bool foundLeftParen = false;
                while (stackTop > 0) {
                    const char* top = stack[--stackTop];
                    if (strcmp(top, "(") == 0) {
                        foundLeftParen = true;
                        break;
                    } else {
                        outputRPN[outCount++] = top;
                    }
                }
                if (!foundLeftParen) {
                    std::cout << "parenthesis mismatch @ " << i << std::endl;
                    return false; // Mismatched parens
                }
            }
            else {
                outputRPN[outCount++] = token; // numbers or variables
            }
        }

        while (stackTop > 0) {
            const char* top = stack[--stackTop];
            if (strcmp(top, "(") == 0 || strcmp(top, ")") == 0) {
                std::cout << "parenthesis mismatch" << std::endl;
                return false;
            } 
            outputRPN[outCount++] = top;
        }

        return true;
    }


    
} // namespace HAL_JSON
