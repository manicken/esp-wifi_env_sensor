#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {

        TokenType GetFundamentalTokenType(const char* str) {
            std::cout << "GetFundamentalTokenType: >>>" << str << "<<<\n";
            if (StrEqualsIC(str, "if")) return TokenType::If;
            else if (StrEqualsIC(str, "endif")) return TokenType::EndIf;
            else if (StrEqualsIC(str, "else")) return TokenType::Else;
            else if (StrEqualsIC(str, "elseif")) return TokenType::ElseIf;
            else if (StrEqualsIC(str, "then")) return TokenType::Then;
            else if (StrEqualsIC(str, "do")) return TokenType::Then;
            else if (StrEqualsIC(str, "on")) return TokenType::On;
            else if (StrEqualsIC(str, "and")) return TokenType::And;
            else if (StrEqualsIC(str, "or")) return TokenType::Or;
            else if (StrEqualsIC(str, ";")) return TokenType::ActionSeparator;
            else if (StrEqualsIC(str, "\\")) return TokenType::ActionJoiner;
            else if (StrEqualsIC(str, "endon")) return TokenType::EndOn;
            else return TokenType::NotSet;
        }
        const char* TokenTypeToString(TokenType type) {
            switch (type) {
                case TokenType::NotSet: return "NotSet";
                case TokenType::On: return "On";
                case TokenType::EndOn: return "EndOn";
                case TokenType::If: return "If";
                case TokenType::EndIf: return "EndIf";
                case TokenType::IfCondition: return "IfCondition";
                case TokenType::Else: return "Else";
                case TokenType::ElseIf: return "ElseIf";
                case TokenType::Then: return "Then";
                case TokenType::And: return "And";
                case TokenType::Or: return "Or";
                case TokenType::ActionSeparator: return "ActionSeparator";
                case TokenType::ActionJoiner: return "ActionJoiner";
                case TokenType::Action: return "Action";
                case TokenType::Merged: return "Merged";
                case TokenType::Ignore: return "Ignore";
                default:
                    return "Unknown";
            }
        }
        
        Token::Token(): type(TokenType::NotSet) {
            Set(nullptr, -1, -1);
        }

        void Token::Set(const char* _text, int _line, int _column) {
            //isAction = false;
            //merged = false;
            //subTokenCount = 0;
            start = _text;
            end = (start!=nullptr)?(start + strlen(start)):nullptr;
            //text = _text;
            line = _line;
            column = _column;
            itemsInBlock = 0;
            if (start!=nullptr) { // start is only nullptr when the token array is first created
                type = GetFundamentalTokenType(start);
                //std::cout << "FundamentalTokenType is set to: " << TokenTypeToString(type) << "\n";
            }
        }
        Token::~Token() {
            // nothing to do here as no dynamic memory is allocated
        }

        void Token::MarkTokenGroup(int size, TokenType constructType) {
            if (size == 1) {
                this->type = constructType;
                //subTokenCount = 0; // set as zero for the moment
                itemsInBlock = 0;
                //if (constructType == TokenType::Action) isAction = true; // TODO, to be removed in favor of using TokenType
                return;
            }

            //subTokenCount = size;
            itemsInBlock = size;
            
            // mark the following and inclusive this tokens merged
            //this[0].merged = true; // TODO should be removed in favor of TokenType
            this[0].type = constructType;
            for (int i = 1; i < size; i++) {
                //this[i].merged = true; // TODO should be removed in favor of TokenType
                
                if (constructType == TokenType::Action) {
                    //isAction = true; // TODO should be removed in favor of TokenType
                    if (this[i].type == TokenType::ActionJoiner) {
                        this[i].type = TokenType::Ignore;
                        continue;
                    }

                } 
                this[i].type = TokenType::Merged;
            }
        }

        bool Token::MergedOrIgnore() const {
            return ((type == TokenType::Merged || type == TokenType::Ignore) && itemsInBlock == 0);
        }

        bool Token::AnyType(const TokenType* candidates) {
            while(*candidates != TokenType::NotSet) {
                if (type == *candidates) return true;
                candidates++;
            }
            return false;
        }

        Tokens::Tokens() : zeroCopy(true), firstTokenStartOffset(nullptr), items(nullptr), count(0) {}

        Tokens::Tokens(int count) : zeroCopy(false), firstTokenStartOffset(nullptr), items(new Token[count]), count(count) { }

        Tokens::~Tokens() {
            if (zeroCopy == false)
                delete[] items;
        }
        std::string Tokens::ToString() {
            std::string str;
            //str += "firstTokenStartOffset:"; str += (firstTokenStartOffset?"true ":"false ");
            for (int i=0;i<count;i++) {
                ZeroCopyString zcStrCopy = items[i];
                if (i==0 && firstTokenStartOffset != nullptr) zcStrCopy.start = firstTokenStartOffset;
                str += zcStrCopy.ToString();
            }
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
            if (_tokens.firstTokenStartOffset != nullptr) {
                msg += "firstTokenStartOffset set, ";
            }
            for (int i=0;i<tokenCount;i++) {
                Token& tok = tokens[i];
                // skip duplicate prints as theese will be printed as the Tokens block
                // ignore types are allways part of a token block 
                if ((tok.type == TokenType::Merged) && !sub) continue; 
                
                std::string msgLine;
                if (sub) msgLine += "    ";
                
                // only print one level down and only if the type is Action or IfCondition
                if (!sub && (tok.type == TokenType::Action || tok.type == TokenType::IfCondition)) {
                    msgLine += "  Tokens block:\n";
                    Tokens tokens;
                    tokens.items = &tok;
                    tokens.count = tok.itemsInBlock;
                    msgLine += PrintTokens(tokens, true);
                } else {
                    msgLine +=
                    "Token(" + std::to_string(i) + "): " +
                    "(line:" + std::to_string(tok.line) + 
                    ", col:" + std::to_string(tok.column) + 
                    ", itemsInBlock:" + std::to_string(tok.itemsInBlock) + 
                    ", type:" + TokenTypeToString(tok.type) +
                    ")";


                    msgLine += " >>> " + tok.ToString() + " <<<";// size: " + std::to_string(tok.Length());// std::string(tok.text);
                }
                msg += msgLine + "\n";
            }
            return msg;
        }
    }
}