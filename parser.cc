#include "parser.hh"

#include <iostream>

using namespace std;

void Parser::syntaxError() {
    cout << "SYNTAX ERROR\n";
    exit(1);
}

void Parser::runtimeError() {
    cout << "RUNTIME ERROR\n";
    exit(1);
}

void Parser::expect(TokenType type) {
    Token t = lexer.GetToken();

    if (t.tokenType != type) syntaxError();
}

void Parser::checkType(Token token, TokenType type) {
    if (token.tokenType != type) syntaxError();
}

void Parser::ParseInput() { parseProgram(); }

void Parser::ReduceAndPrint() {
    for (int i = 0; i < reductions.size(); i++) {
        Term *reduction = reductions[i];
        bool reduce = true;

        while (reduce) {
            reduce = false;
            reduce |= substituteDefs(reduction);
            reduce |= betaReduce(reduction);
        }

        switch (printTypes[i]) {
            case PRINT_FUNC:
                cout << termToString(reduction) << endl;
                break;
            case PRINT_BOOL:
                cout << termToBool(reduction) << endl;
                break;
            case PRINT_NUM:
                cout << termToNum(reduction) << endl;
                break;
        }
    }
}

void Parser::parseProgram() {
    Token t = lexer.Peek();

    if (t.tokenType == LET) parseDefList();

    parseReductionList();
    expect(END_OF_FILE);
}

void Parser::parseDefList() {
    parseDef();

    Token t = lexer.Peek();
    if (t.tokenType == LET) parseDefList();
}

void Parser::parseDef() {
    expect(LET);

    Token t = lexer.GetToken();
    checkType(t, ID);
    string var = t.lexeme;

    expect(EQUAL);

    Term *term = parseTerm();
    definitions[var] = term;

    expect(SEMICOLON);
}

void Parser::parseReductionList() {
    parseReduction();

    Token t = lexer.Peek();

    if (t.tokenType != END_OF_FILE) {
        parseReductionList();
    }
}

void Parser::parseReduction() {
    PrintType printType = parsePrint();
    printTypes.push_back(printType);

    Term *term = parseTerm();
    reductions.push_back(term);

    expect(SEMICOLON);
}

PrintType Parser::parsePrint() {
    Token t = lexer.GetToken();
    checkType(t, PRINT);

    if (t.lexeme.compare("print") == 0)
        return PRINT_FUNC;
    else if (t.lexeme.compare("printnum") == 0)
        return PRINT_NUM;
    else if (t.lexeme.compare("printbool") == 0)
        return PRINT_BOOL;
}

Term *Parser::parseTerm() {
    Token t = lexer.Peek();
    Term *term = new Term;
    term->var = "";
    term->lTerm = NULL;
    term->rTerm = NULL;

    if (t.tokenType == LAMBDA) {
        term->type = ABSTRACTION;

        expect(LAMBDA);

        t = lexer.GetToken();
        checkType(t, ID);
        term->var = t.lexeme;

        expect(DOT);

        term->lTerm = parseTerm();
    } else if (t.tokenType == LPAREN) {
        term->type = APPLICATION;

        expect(LPAREN);
        term->lTerm = parseTerm();
        expect(RPAREN);

        t = lexer.Peek();

        if (t.tokenType != RPAREN && t.tokenType != SEMICOLON &&
            t.tokenType != END_OF_FILE) {
            term->rTerm = parseTerm();
        }
    } else if (t.tokenType == ID || t.tokenType == NUM) {
        term->type = PRIMARY;
        term->var = parsePrimary();
        t = lexer.Peek();

        if (t.tokenType != RPAREN && t.tokenType != SEMICOLON &&
            t.tokenType != END_OF_FILE) {
            term->rTerm = parseTerm();
        }
    } else
        syntaxError();

    return term;
}

string Parser::parsePrimary() {
    Token t = lexer.GetToken();

    if (t.tokenType != ID && t.tokenType != NUM) syntaxError();

    return t.lexeme;
}

void Parser::parseComment() {}

bool Parser::betaReduce(Term *term) {
    if (term == NULL) return false;
    bool changed = false;

    if (term->type == APPLICATION) {
        if (term->rTerm == NULL) {
            if (term->lTerm->type == APPLICATION) {
                Term *oldTerm = term->lTerm;
                term->lTerm = oldTerm->lTerm;
                term->rTerm = oldTerm->rTerm;
                delete oldTerm;
                changed = true;

                betaReduce(term->rTerm);
            }

            changed |= betaReduce(term->lTerm);
        } else if (term->lTerm->type != ABSTRACTION) {
            if (term->lTerm->type == APPLICATION &&
                term->lTerm->rTerm == NULL) {
                Term *oldTerm = term->lTerm;
                term->lTerm = oldTerm->lTerm;
                delete oldTerm;
                changed = true;
            }

            changed |= betaReduce(term->lTerm);
            changed |= betaReduce(term->rTerm);
        } else {
            Term *application = term;
            Term *abstraction = application->lTerm;
            Term *arg = application->rTerm;
            string var = abstraction->var;
            Term *termToSub, *oldLTerm, *oldRTerm;

            if (arg->type == ABSTRACTION) {
                termToSub = arg;
                application->rTerm = NULL;
            } else if (arg->type == APPLICATION) {
                termToSub = arg->lTerm;
                application->rTerm = arg->rTerm;
            } else if (arg->type == PRIMARY) {
                termToSub = new Term;
                termToSub->type = PRIMARY;
                termToSub->var = arg->var;
                termToSub->lTerm = NULL;
                termToSub->rTerm = NULL;
                application->rTerm = arg->rTerm;
            }

            map<string, bool> freeVars;
            map<string, string> renames;
            getFreeVars(termToSub, freeVars);
            alphaRename(abstraction, freeVars, renames);
            substituteVars(abstraction->lTerm, abstraction->var, termToSub);

            application->lTerm = abstraction->lTerm;
            delete abstraction;
            delete arg;
            changed |= true;
        }
    } else if (term->type == PRIMARY) {
        if (term->rTerm != NULL) {
            changed |= betaReduce(term->rTerm);
        }
    }

    return changed;
}

void Parser::getFreeVars(Term *term, map<string, bool> &freeVars) {
    if (term == NULL) return;
    map<string, bool> freeVarsCopy;

    switch (term->type) {
        case ABSTRACTION:
            freeVarsCopy = freeVars;
            freeVarsCopy[term->var] = false;
            getFreeVars(term->lTerm, freeVarsCopy);

            for (auto i = freeVarsCopy.begin(); i != freeVarsCopy.end(); i++)
                if (i->second) freeVars[i->first] = true;
            break;
        case APPLICATION:
            getFreeVars(term->lTerm, freeVars);
            getFreeVars(term->rTerm, freeVars);
            break;
        case PRIMARY:
            if (freeVars.find(term->var) == freeVars.end())
                freeVars[term->var] = true;
            break;
    }
}

void Parser::alphaRename(Term *term, const map<string, bool> &freeVars,
                         map<string, string> &renames) {
    if (term == NULL) return;
    map<string, string> renamesCopy;
    string renamedVar;

    switch (term->type) {
        case ABSTRACTION:
            if (freeVars.find(term->var) == freeVars.end()) {
                alphaRename(term->lTerm, freeVars, renames);
            } else {
                renamedVar = nextRenamedVar();
                renamesCopy = renames;
                renamesCopy[term->var] = renamedVar;
                term->var = renamedVar;
                alphaRename(term->lTerm, freeVars, renamesCopy);
            }
            break;
        case APPLICATION:
            alphaRename(term->lTerm, freeVars, renames);
            alphaRename(term->rTerm, freeVars, renames);
            break;
        case PRIMARY:
            if (renames.find(term->var) != renames.end())
                term->var = renames[term->var];
            break;
    }
    return;
}

bool Parser::substituteDefs(Term *term) {
    if (term == NULL) return false;
    bool changed = false;

    if (term->type == ABSTRACTION)
        changed = substituteDefs(term->lTerm);
    else if (term->type == APPLICATION) {
        changed = substituteDefs(term->lTerm);
        changed |= substituteDefs(term->rTerm);
    } else if (term->type == PRIMARY) {
        if (definitions.find(term->var) != definitions.end()) {
            term->type = APPLICATION;
            term->lTerm = copyTerm(definitions[term->var]);
            term->var = "";
            changed = true;
        }

        changed |= substituteDefs(term->rTerm);
    }

    return changed;
}

void Parser::substituteVars(Term *term, string var, Term *termToSub) {
    if (term == NULL) return;

    if (term->type == ABSTRACTION) {
        if (term->var.compare(var) != 0) {
            substituteVars(term->lTerm, var, termToSub);
        }
    } else if (term->type == APPLICATION) {
        substituteVars(term->lTerm, var, termToSub);

        if (term->rTerm != NULL) {
            substituteVars(term->rTerm, var, termToSub);
        }
    } else if (term->type == PRIMARY) {
        if (term->var.compare(var) == 0) {
            term->type = APPLICATION;
            term->lTerm = copyTerm(termToSub);
        }

        if (term->rTerm != NULL) {
            substituteVars(term->rTerm, var, termToSub);
        }
    }
}

string Parser::nextRenamedVar() { return "a" + to_string(renameCount++); }

Term *Parser::copyTerm(Term *term) {
    if (term == NULL) return NULL;

    Term *newTerm = new Term;
    newTerm->type = term->type;
    newTerm->var = term->var;
    newTerm->lTerm = copyTerm(term->lTerm);
    newTerm->rTerm = copyTerm(term->rTerm);

    return newTerm;
}

string Parser::termToString(Term *term) {
    if (term == NULL) return "";

    if (term->type == ABSTRACTION) {
        return ("!" + term->var + "." + termToString(term->lTerm));
    } else if (term->type == APPLICATION) {
        return ("(" + termToString(term->lTerm) + ")" +
                termToString(term->rTerm));
    } else if (term->type == PRIMARY) {
        string res = term->var;
        if (term->rTerm != NULL) res += " " + termToString(term->rTerm);
        return res;
    }
}

bool Parser::termToBool(Term *term) {
    if (term == NULL) runtimeError();

    if (term->type == PRIMARY)
        runtimeError();
    else if (term->type == APPLICATION) {
        if (term->lTerm->type != ABSTRACTION || term->rTerm != NULL)
            runtimeError();

        term = term->lTerm;
    }

    if (term->lTerm->type != ABSTRACTION || term->lTerm->lTerm->type != PRIMARY)
        runtimeError();

    string varA = term->var;
    string varB = term->lTerm->var;
    string varC = term->lTerm->lTerm->var;

    if (varA.compare(varC) == 0)
        return true;
    else if (varB.compare(varC) == 0)
        return false;
    else
        runtimeError();
}

int Parser::termToNum(Term *term) { return 0; }