#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {

        
        Token::Token() {
            Set(nullptr, -1, -1);
        }

        void Token::Set(const char* _text, int _line, int _column) {
            isAction = false;
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

        void ReportTokenInfo(const Token& t, const char* msg, const char* param) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #ifdef _WIN32
            std::cout << "Info " << message << std::endl;
    #else
            //GlobalLogger.Info(F("Token:"), message.c_str());
    #endif
        }
        void ReportTokenError(const Token& t, const char* msg, const char* param) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #ifdef _WIN32
            std::cerr << "Error " << message << std::endl;
    #else
            GlobalLogger.Error(F("Token:"), message.c_str());
    #endif
        }
        void ReportTokenWarning(const Token& t, const char* msg, const char* param) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #ifdef _WIN32
            std::cout << "Warning " << message << std::endl;
    #else
            GlobalLogger.Warn(F("Token:"), message.c_str());
    #endif
        }

        std::string PrintTokens(Tokens& _tokens, bool sub) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            for (int i=0;i<tokenCount;i++) {
                Token& tok = tokens[i];
                if (tok.Merged() && !sub) continue;
                //if (tok.EqualsIC("and")) continue;
                //if (tok.EqualsIC(";")) continue;
                std::string msgLine;
                if (sub) msgLine += "    ";
                //char buffer[64];
                //snprintf(buffer, sizeof(buffer), "addr: %p", (void*)&tokens[i]);
                //std::string memaddr = buffer;
                msgLine +=
                    //memaddr + "  " + 
                    "Token(" + std::to_string(i) + "): " +
                    "(line:" + std::to_string(tok.line) + 
                    ", col:" + std::to_string(tok.column) + 
                    ", itemCount:" + std::to_string(tok.itemsInBlock) + 
                    ", merged:" + (tok.merged?"true":"false") + 
                    ", isAction:" + (tok.isAction?"true":"false") + 
                    ", subTokenCount:" + std::to_string(tok.subTokenCount) + 
                    ")";
                if (!sub && tok.subTokenCount > 0) {
                    msgLine += "\n  subTokens:\n";
                    Tokens tokens;
                    tokens.items = &tok;
                    tokens.count = tok.subTokenCount;
                    msgLine += PrintTokens(tokens, true);
                } else {
                    msgLine += " >>> " + tok.ToString() + " <<<";// size: " + std::to_string(tok.Length());// std::string(tok.text);
                }
                msg += msgLine + "\n";
            }
            return msg;
        }
    }
}