#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <vector>

#include "input.hh"

typedef enum {
    END_OF_FILE = 0,
    ERROR,
    LET,
    PRINT,
    EQUAL,
    SEMICOLON,
    LAMBDA,
    DOT,
    LPAREN,
    RPAREN,
    ID,
    NUM,
} TokenType;

class Token {
   public:
    void Print();

    std::string lexeme;
    TokenType tokenType;
    int lineNum;
};

class Lexer {
   public:
    Token GetToken();
    TokenType UngetToken(Token);
    Token Peek();
    void Expect(TokenType);
    Lexer();

   private:
    std::vector<Token> tokens;
    int lineNum;
    Token tmp;
    Input input;

    bool skipSpace();
    bool isKeyword(std::string);
    TokenType findKeywordTokenType(std::string);
    Token scanNumber();
    Token scanIdOrKeyword();
};

#endif
