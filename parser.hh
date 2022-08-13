#ifndef __PARSER_H__
#define __PARSER_H__

#include <map>
#include <string>
#include <vector>

#include "lexer.hh"

using namespace std;

typedef enum { ABSTRACTION = 0, APPLICATION, PRIMARY } TermType;
typedef enum { PRINT_FUNC = 0, PRINT_NUM, PRINT_BOOL } PrintType;

struct Term {
    TermType type;
    string var;
    Term *lTerm;
    Term *rTerm;
};

class Parser {
   public:
    void ParseInput();
    void ReduceAndPrint();

   private:
    Lexer lexer;
    map<string, Term *> definitions;
    vector<Term *> reductions;
    vector<int> printTypes;
    int renameCount;

    void syntaxError();
    void runtimeError();
    void expect(TokenType type);
    void checkType(Token token, TokenType type);
    void parseProgram();
    void parseDefList();
    void parseDef();
    void parseReductionList();
    void parseReduction();
    PrintType parsePrint();
    Term *parseTerm();
    string parsePrimary();
    bool termToBool(Term *term);
    int termToNum(Term *term);
    void parseComment();
    bool betaReduce(Term *term);
    void getFreeVars(Term *term, map<string, bool> &freeVars);
    void alphaRename(Term *term, const map<string, bool> &freeVars,
                     map<string, string> &renames);
    bool substituteDefs(Term *term);
    void substituteVars(Term *term, string var, Term *termToSub);
    string nextRenamedVar();
    Term *copyTerm(Term *term);
    string termToString(Term *term);
};

#endif
