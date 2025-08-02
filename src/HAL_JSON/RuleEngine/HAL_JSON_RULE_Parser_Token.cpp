#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {
        Token::Token() {
            Set(nullptr, -1, -1);
        }
        void Token::Set(const char* _text, int _line, int _column) {
            merged = false;
            subTokens = nullptr;
            subTokenCount = 0;
            text = _text;
            line = _line;
            column = _column;
            itemsInBlock = 0;
        }
        Token::~Token() {
            // nothing to do here as no dynamic memory is allocated
        }

        void Token::InitSubTokens(int size) {
            subTokenCount = size;
            // just refer to this ptr as the first item
            // subTokenCount refers to the tokens that 'belong' to the 
            subTokens = this; 
            // mark the tokens merged
            for (int i = 0; i < subTokenCount; i++) {
                subTokens[i].merged = true;
            }
        }

        bool Token::Merged() {
            return (merged && subTokenCount == 0);
        }
    }
}