#include "lexer.hh"

#include <cctype>
#include <iostream>
#include <istream>
#include <map>
#include <string>
#include <vector>

#include "input.hh"

using namespace std;

string reserved[] = {"END_OF_FILE", "ERROR",  "LET", "PRINT",  "EQUAL",
                     "SEMICOLON",   "LAMBDA", "DOT", "LPAREN", "RPAREN",
                     "ID",          "NUM",    "BOOL"};

map<string, int> keywords = {{"let", 2},       {"print", 3}, {"printnum", 3},
                             {"printbool", 3}, {"true", 12}, {"false", 12}};

void Token::Print() {
    cout << "{" << this->lexeme << " , " << reserved[(int)this->tokenType]
         << " , " << this->lineNum << "}\n";
}

Lexer::Lexer() {
    this->lineNum = 1;
    this->tmp.lexeme = "";
    this->tmp.lineNum = 1;
    this->tmp.tokenType = ERROR;
}

bool Lexer::skipSpace() {
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    lineNum += (c == '\n');

    while (!input.AtEnd() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        lineNum += (c == '\n');
    }

    if (!input.AtEnd()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool Lexer::isKeyword(string s) { return keywords.find(s) != keywords.end(); }

TokenType Lexer::findKeywordTokenType(string s) {
    if (!isKeyword(s)) return ERROR;

    return (TokenType)keywords[s];
}

Token Lexer::scanNumber() {
    char c;

    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";

            while (!input.AtEnd() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }

            if (!input.AtEnd()) {
                input.UngetChar(c);
            }
        }

        tmp.tokenType = NUM;
        tmp.lineNum = lineNum;

        return tmp;
    } else {
        if (!input.AtEnd()) {
            input.UngetChar(c);
        }

        tmp.lexeme = "";
        tmp.tokenType = ERROR;
        tmp.lineNum = lineNum;

        return tmp;
    }
}

Token Lexer::scanIdOrKeyword() {
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";

        while (!input.AtEnd() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }

        if (!input.AtEnd()) {
            input.UngetChar(c);
        }

        tmp.lineNum = lineNum;

        if (isKeyword(tmp.lexeme))
            tmp.tokenType = findKeywordTokenType(tmp.lexeme);
        else
            tmp.tokenType = ID;
    } else {
        if (!input.AtEnd()) {
            input.UngetChar(c);
        }

        tmp.lexeme = "";
        tmp.tokenType = ERROR;
    }

    return tmp;
}

TokenType Lexer::UngetToken(Token tok) {
    tokens.push_back(tok);
    return tok.tokenType;
}

Token Lexer::GetToken() {
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        this->tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    skipSpace();
    this->tmp.lexeme = "";
    this->tmp.lineNum = this->lineNum;
    this->tmp.tokenType = END_OF_FILE;

    if (!input.AtEnd())
        input.GetChar(c);
    else
        return tmp;

    switch (c) {
        case '=':
            tmp.tokenType = EQUAL;
            return tmp;
        case ';':
            tmp.tokenType = SEMICOLON;
            return tmp;
        case '!':
            tmp.tokenType = LAMBDA;
            return tmp;
        case '.':
            tmp.tokenType = DOT;
            return tmp;
        case '(':
            tmp.tokenType = LPAREN;
            return tmp;
        case ')':
            tmp.tokenType = RPAREN;
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return scanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return scanIdOrKeyword();
            } else if (input.AtEnd())
                tmp.tokenType = END_OF_FILE;
            else
                tmp.tokenType = ERROR;

            return tmp;
    }
}

Token Lexer::Peek() {
    Token t = GetToken();
    UngetToken(t);
    return t;
}