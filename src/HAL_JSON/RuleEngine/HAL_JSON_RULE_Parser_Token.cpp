#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {
        Token::Token() {
            Set(nullptr, -1, -1);
        }
        void Token::Set(const char* _text, int _line, int _column) {
            merged = false;
            subTokens.items = nullptr;
            subTokens.count = 0;
            text = _text;
            line = _line;
            column = _column;
            itemsInBlock = 0;
        }
        Token::~Token() {
            // nothing to do here as no dynamic memory is allocated
        }

        void Token::InitSubTokens(int size) {
            // just refer to this ptr as the first item
            // subTokenCount refers to the tokens that 'belong' to the subtokens instance
            subTokens.items = this;

            subTokens.count = size;
            
            // mark the tokens merged
            for (int i = 0; i < size; i++) {
                subTokens.items[i].merged = true;
            }
        }

        bool Token::Merged() {
            return (merged && subTokens.count == 0);
        }
        Tokens::Tokens() : zeroCopy(true), items(nullptr), count(0) {}

        Tokens::Tokens(int count) : count(count) {
            items = new Token[count];
            zeroCopy = false;
        }

        Tokens::~Tokens() {
            if (zeroCopy == false)
                delete[] items;
        }
        std::string Tokens::ToString() {
            std::string str;
            for (int i=0;i<count;i++)
                str += items[i].text;
            return str;
        }
    }
}