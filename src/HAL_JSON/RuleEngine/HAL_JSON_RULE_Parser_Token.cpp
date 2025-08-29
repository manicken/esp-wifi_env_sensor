#include "HAL_JSON_RULE_Parser_Token.h"

namespace HAL_JSON {
    namespace Rules {

        TokenType GetFundamentalTokenType(const char* str) {
        #ifdef _WIN32
            std::cout << "GetFundamentalTokenType: >>>" << str << "<<<\n";
        #endif
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
                default: return "Unknown";
            }
        }

        const char* ExpTokenTypeToString(ExpTokenType type) {
            switch (type) {
                case ExpTokenType::NotSet: return "NotSet";
                case ExpTokenType::LeftParenthesis: return "LeftParenthesis";
                case ExpTokenType::RightParenthesis: return "RightParenthesis";
                case ExpTokenType::LogicalAnd: return "LogicalAnd";
                case ExpTokenType::LogicalOr: return "LogicalOr";
                case ExpTokenType::CompareEqualsTo: return "CompareEqualsTo";
                case ExpTokenType::CompareNotEqualsTo: return "CompareNotEqualsTo";
                case ExpTokenType::CompareLessThan: return "CompareLessThan";
                case ExpTokenType::CompareGreaterThan: return "CompareGreaterThan";
                case ExpTokenType::CompareLessThanOrEqual: return "CompareLessThanOrEqual";
                case ExpTokenType::CompareGreaterThanOrEqual: return "CompareGreaterThanOrEqual";
                case ExpTokenType::CalcPlus: return "CalcPlus";
                case ExpTokenType::CalcMinus: return "CalcMinus";
                case ExpTokenType::CalcMultiply: return "CalcMultiply";
                case ExpTokenType::CalcDivide: return "CalcDivide";
                case ExpTokenType::CalcModulus: return "CalcModulus";
                case ExpTokenType::CalcBitwiseAnd: return "CalcBitwiseAnd";
                case ExpTokenType::CalcBitwiseOr: return "CalcBitwiseOr";
                case ExpTokenType::CalcBitwiseExOr: return "CalcBitwiseExOr";
                case ExpTokenType::CalcBitwiseLeftShift: return "CalcBitwiseLeftShift";
                case ExpTokenType::CalcBitwiseRightShift: return "CalcBitwiseRightShift";
                case ExpTokenType::Operand: return "Operand";
                default: return "Unknown";
            }
        }

        ExpressionToken::ExpressionToken() : type(ExpTokenType::NotSet) {

        }
        ExpressionToken::ExpressionToken(const char* _start, const char* _end, ExpTokenType _type) {
            start = _start;
            end = _end;
            type = _type;
        }
        ExpressionToken::ExpressionToken(const char* _start, int length, ExpTokenType _type) {
            start = _start;
            end = _start + length;
            type = _type;
        }
        ExpressionToken::~ExpressionToken() {
            // nothing to free here
        }
        bool ExpressionToken::AnyType(const ExpTokenType* candidates) {
            while(*candidates != ExpTokenType::NotSet) {
                if (type == *candidates) return true;
                candidates++;
            }
            return false;
        }
        ExpressionTokens::ExpressionTokens() {
            items = nullptr;
            count = 0;
            index = 0;
        }
        ExpressionTokens::ExpressionTokens(int _count) {
            count = _count;
            items = new ExpressionToken*[_count];
            for (int i=0;i<count;i++)
                items[i] = nullptr;
            
            index = 0;
        }
        ExpressionTokens::~ExpressionTokens() {
            if (items != nullptr) {
                for (int i=0;i<count;i++)
                    delete items[i];
                delete[] items;
                items = nullptr;
            }
        }
        
        Token::Token(): type(TokenType::NotSet) {
            Set(nullptr, -1, -1);
        }

        void Token::Set(const char* _text, int _line, int _column) {
            start = _text;
            end = (start!=nullptr)?(start + strlen(start)):nullptr;
            line = _line;
            column = _column;
            itemsInBlock = 0;
            hasElse = 0;
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
                itemsInBlock = 0;
                return;
            }
            itemsInBlock = size;
            
            // mark the following and inclusive this tokens merged
            this[0].type = constructType;
            for (int i = 1; i < size; i++) {

                if (constructType == TokenType::Action) {
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

        Tokens::Tokens() : zeroCopy(true), firstTokenStartOffset(nullptr), items(nullptr), count(0), rootBlockCount(0), currIndex(0) {}

        Tokens::Tokens(int count) : zeroCopy(false), firstTokenStartOffset(nullptr), items(new Token[count]), count(count), rootBlockCount(0), currIndex(0) { }

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

        std::string PrintTokens(Tokens& _tokens, int subTokenIndexOffset) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (subTokenIndexOffset == 0)
                msg += "rootLevelBlockCount: " + std::to_string(_tokens.rootBlockCount) + "\n";

            if (_tokens.firstTokenStartOffset != nullptr) {
                msg += "firstTokenStartOffset set, ";
            }
            bool lastWasBlock = false;
            for (int i=0;i<tokenCount;i++) {
                Token& tok = tokens[i];
                std::string msgLine;
                if (subTokenIndexOffset > 0) msgLine += "  ";
                // skip duplicate prints as theese will be printed as the Tokens block
                // ignore types are allways part of a token block 
                if ((tok.type == TokenType::Merged) && subTokenIndexOffset == 0) {
                    //msgLine += " skipping: ";
                    continue; 
                }
                
                
                
                // only print one level down and only if the type is Action or IfCondition
                if (subTokenIndexOffset == 0 && tok.itemsInBlock > 0 && (tok.type == TokenType::Action || tok.type == TokenType::IfCondition)) {
                    if (lastWasBlock == false)
                        msgLine += "\n";
                    msgLine += "Tokens block:\n";
                    Tokens tokens;
                    tokens.items = &tok;
                    tokens.count = tok.itemsInBlock;
                    msgLine += PrintTokens(tokens, i);
                    lastWasBlock = true;
                } else {
                    msgLine +=
                    "Token(" + std::to_string(i+subTokenIndexOffset) + "): " +
                    "(line:" + std::to_string(tok.line) + 
                    ", col:" + std::to_string(tok.column) + 
                    ", count:" + std::to_string(tok.itemsInBlock) + 
                    ", type:" + TokenTypeToString(tok.type) +
                    ((tok.type == TokenType::If)?((tok.hasElse==1)?", hasElse:true":", hasElse:false"):"")+
                    "): ";

                    lastWasBlock = false;
                    //msgLine += " >>> " + tok.ToString() + " <<<";// size: " + std::to_string(tok.Length());// std::string(tok.text);
                    msgLine += tok.ToString();
                }
                msg += msgLine + "\n";
            }
            return msg;
        }

        std::string PrintExpressionTokens(ExpressionTokens& _tokens, int start, int end) {
            ExpressionToken** tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (end == -1) end = tokenCount;
 
            for (int i=start;i<end;i++) {
                ExpressionToken& tok = *tokens[i];
                std::string msgLine = "Token[" + std::to_string(i) + "]  ";
                if (tok.type != ExpTokenType::Operand) {
                    msgLine += "---";
                    msgLine += ExpTokenTypeToString(tok.type);
                    msgLine += "---";
                } else {
                    msgLine += tok.ToString();
                }
                msg += msgLine + "\n";
            }
            return msg;
        }
    }
}