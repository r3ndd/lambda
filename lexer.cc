#include "lexer.hh"

#include <cctype>
#include <iostream>
#include <istream>
#include <map>
#include <string>
#include <vector>

#include "input.hh"

using namespace std;

string reserved[] = {"END_OF_FILE", "ERROR",     "NEWLINE", "LET", "PRINT",
                     "EQUAL",       "SEMICOLON", "LAMBDA",  "DOT", "LPAREN",
                     "RPAREN",      "ID",        "NUM",     "BOOL"};

map<string, int> keywords = {{"let", 2},       {"print", 3}, {"printnum", 3},
                             {"printbool", 3}, {"true", 12}, {"false", 12}};

map<string, int> specialSequences = {{"/*", 13}, {"*/", 14}};

void Token::Print() {
    cout << "{" << lexeme << " , " << reserved[(int)tokenType] << " , "
         << lineNum << "}\n";
}

Lexer::Lexer() {
    lineNum = 1;
    tmp.lexeme = "";
    tmp.lineNum = 1;
    tmp.tokenType = ERROR;
}

bool Lexer::skipSpace() {
    char c;
    bool spaceEncountered = false;

    input.GetChar(c);
    lineNum += (c == '\n');

    while (!input.AtEnd() && isspace(c)) {
        spaceEncountered = true;
        input.GetChar(c);
        lineNum += (c == '\n');
    }

    if (!input.AtEnd()) {
        input.UngetChar(c);
    }

    return spaceEncountered;
}

bool Lexer::isKeyword(string s) { return keywords.find(s) != keywords.end(); }

TokenType Lexer::findKeywordTokenType(string s) {
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

    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    skipSpace();
    tmp.lexeme = "";
    tmp.lineNum = lineNum;
    tmp.tokenType = END_OF_FILE;

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
        case '/':
            if (input.AtEnd()) {
                tmp.tokenType = ERROR;
                return tmp;
            }

            input.GetChar(c);

            if (c != '*' || input.AtEnd()) {
                tmp.tokenType = ERROR;
                return tmp;
            }

            while (true) {
                input.GetChar(c);

                if (input.AtEnd()) {
                    tmp.tokenType = ERROR;
                    return tmp;
                }

                if (c != '*') continue;
                while (c == '*' && !input.AtEnd()) input.GetChar(c);
                if (c == '/') break;

                if (input.AtEnd()) {
                    tmp.tokenType = ERROR;
                    return tmp;
                }
            }

            return GetToken();
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return scanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return scanIdOrKeyword();
            } else if (input.AtEnd()) {
                tmp.tokenType = END_OF_FILE;
                return tmp;
            } else {
                tmp.tokenType = ERROR;
                return tmp;
            }
    }
}

Token Lexer::Peek() {
    Token t = GetToken();
    UngetToken(t);
    return t;
}