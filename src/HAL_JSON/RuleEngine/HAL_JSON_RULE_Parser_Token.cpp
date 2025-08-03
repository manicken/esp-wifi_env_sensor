#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {

        
        Token::Token() {
            Set(nullptr, -1, -1);
        }

        void Token::Set(const char* _text, int _line, int _column) {
            merged = false;
            subTokenCount = 0;
            start = _text;
            end = (start!=nullptr)?(start + strlen(start)):nullptr;
            //text = _text;
            line = _line;
            column = _column;
            itemsInBlock = 0;
        }
        Token::~Token() {
            // nothing to do here as no dynamic memory is allocated
        }

        void Token::InitSubTokens(int size) {
            subTokenCount = size;
            
            // mark the following and inclusive this tokens merged
            for (int i = 0; i < size; i++) {
                this[i].merged = true;
            }
        }

        bool Token::Merged() {
            return (merged && subTokenCount == 0);
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
                str += items[i].ToString();
            return str;
        }

        void ReportTokenInfo(const char* msg, const Token& t) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
    #ifdef _WIN32
            std::cout << "Info " << message << std::endl;
    #else
            //GlobalLogger.Info(F("Token:"), message.c_str());
    #endif
        }
        void ReportTokenError(const char* msg, const Token& t) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
    #ifdef _WIN32
            std::cerr << "Error " << message << std::endl;
    #else
            GlobalLogger.Error(F("Token:"), message.c_str());
    #endif
        }
        void ReportTokenWarning(const char* msg, const Token& t) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
    #ifdef _WIN32
            std::cout << "Warning " << message << std::endl;
    #else
            GlobalLogger.Warn(F("Token:"), message.c_str());
    #endif
        }
    }
}