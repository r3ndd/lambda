
#include "parser.hh"

#include <fstream>
#include <iostream>

#include "libraries.hh"

using namespace std;

void Parser::importError(string msg) {
    cout << "IMPORT ERROR: " << msg << "\n";
    exit(1);
}

void Parser::syntaxError(int lineNum, string msg) {
    cout << "SYNTAX ERROR (" << lineNum << "): " << msg << "\n";
    exit(1);
}

void Parser::runtimeError(string msg) {
    cout << "RUNTIME ERROR: " << msg << "\n";
    exit(1);
}

void Parser::expect(TokenType type, string msg) {
    Token t = lexer.GetToken();
    if (t.tokenType != type) syntaxError(t.lineNum, msg);
}

void Parser::checkType(Token token, TokenType type, string msg) {
    if (token.tokenType != type) syntaxError(token.lineNum, msg);
}

bool Parser::OpenFile(string filename) { return lexer.OpenFile(filename); }

void Parser::ParseInput() { parseProgram(); }

void Parser::ReduceAndPrint() {
    for (long unsigned int i = 0; i < reductions.size(); i++) {
        Term *reduction = reductions[i];
        bool reduce = true;

        while (reduce) {
            reduce = false;
            reduce |= substituteDefs(reduction);
            reduce |= betaReduce(reduction);
            reduce |= collapseParentheses(reduction);
        }

        switch (printTypes[i]) {
            case PRINT_FUNC:
                cout << termToString(reduction) << endl;
                break;
            case PRINT_BOOL:
                cout << (termToBool(reduction) ? "true" : "false") << endl;
                break;
            case PRINT_NUM:
                cout << termToNum(reduction) << endl;
                break;
        }
    }
}

void Parser::parseProgram() {
    Token t = lexer.Peek();
    if (t.tokenType == LET || t.tokenType == IMPORT) parseDefList();

    parseReductionList();
    expect(END_OF_FILE, "Expected end of file");
}

void Parser::parseDefList() {
    parseDef();

    Token t = lexer.Peek();
    if (t.tokenType == LET || t.tokenType == IMPORT) parseDefList();
}

void Parser::parseDef() {
    Token t = lexer.Peek();

    if (t.tokenType == IMPORT) {
        parseImport();
    } else {
        expect(LET, "Expected 'let'");

        Token t = lexer.GetToken();
        checkType(t, ID, "Expected definition name");
        string var = t.lexeme;

        expect(EQUAL, "Expected '='");

        Term *term = parseTerm();
        definitions[var] = term;

        expect(SEMICOLON, "Expected semicolon");
    }
}

void Parser::parseImport() {
    string importName;
    bool isFile;

    expect(IMPORT, "Expected 'import'");

    Token id = lexer.GetToken();
    checkType(id, ID, "Expected import name");

    Token t = lexer.Peek();
    if (t.tokenType == DOT) {
        lexer.UngetToken(id);
        importName = parseFilename();
        isFile = true;
    } else {
        importName = id.lexeme;
        isFile = false;
    }

    expect(SEMICOLON, "Expected semicolon");

    if (isFile) {
        ifstream file;
        file.open(importName);

        if (!file.is_open())
            importError(importName + " not found in local directory");

        string fileData = "";
        char c;

        while (file.get(c)) fileData += c;
        lexer.UnshiftString(fileData);
    } else {
        if (LIBRARIES.find(importName) == LIBRARIES.end())
            importError(importName + " is not a native library");

        lexer.UnshiftString(LIBRARIES.at(importName));
    }
}

string Parser::parseFilename() {
    Token id = lexer.GetToken();
    checkType(id, ID, "Expected import file name");

    expect(DOT, "Expected '.");

    Token ext = lexer.GetToken();
    checkType(ext, HEADER_EXTENSION, "Expected header extension");

    return id.lexeme + "." + ext.lexeme;
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

    expect(SEMICOLON, "Expected semicolon");
}

PrintType Parser::parsePrint() {
    Token t = lexer.GetToken();
    checkType(t, PRINT, "Expected print type");

    if (t.lexeme.compare("print") == 0)
        return PRINT_FUNC;
    else if (t.lexeme.compare("printnum") == 0)
        return PRINT_NUM;
    else if (t.lexeme.compare("printbool") == 0)
        return PRINT_BOOL;

    syntaxError(t.lineNum, "Invalid print type");
}

Term *Parser::parseTerm() {
    Token t = lexer.Peek();
    Term *term = new Term;
    term->var = "";
    term->lTerm = NULL;
    term->rTerm = NULL;

    switch (t.tokenType) {
        case LAMBDA:
            term->type = ABSTRACTION;

            expect(LAMBDA, "Expected '!'");

            t = lexer.GetToken();
            checkType(t, ID, "Expected variable name");
            term->var = t.lexeme;

            expect(DOT, "Expected '.'");

            term->lTerm = parseTerm();
            break;
        case LPAREN:
            term->type = APPLICATION;

            expect(LPAREN, "Expected ')'");
            term->lTerm = parseTerm();
            expect(RPAREN, "Expected ')'");

            t = lexer.Peek();

            if (t.tokenType != RPAREN && t.tokenType != SEMICOLON &&
                t.tokenType != END_OF_FILE) {
                term->rTerm = parseTerm();
            }
            break;
        case ID:
        case NUM:
            if (t.tokenType == ID) {
                term->type = PRIMARY;
                term->var = parsePrimary();
            } else {
                term->type = APPLICATION;
                term->lTerm = numToTerm(stoi(parsePrimary()));
            }

            t = lexer.Peek();

            if (t.tokenType != RPAREN && t.tokenType != SEMICOLON &&
                t.tokenType != END_OF_FILE) {
                term->rTerm = parseTerm();
            }
            break;
        default:
            syntaxError(t.lineNum, "Unable to parse term");
    }

    return term;
}

string Parser::parsePrimary() {
    Token t = lexer.GetToken();
    if (t.tokenType != ID && t.tokenType != NUM)
        syntaxError(t.lineNum, "Primary must be alphanumeric");
    return t.lexeme;
}

void Parser::parseComment() {}

bool Parser::betaReduce(Term *term) {
    if (term == NULL) return false;
    bool changed = false;

    switch (term->type) {
        case APPLICATION:
            if (term->rTerm == NULL) {
                changed |= betaReduce(term->lTerm);
            } else if (term->lTerm->type != ABSTRACTION) {
                changed |= betaReduce(term->lTerm);
                changed |= betaReduce(term->rTerm);
            } else {
                Term *application = term;
                Term *abstraction = application->lTerm;
                Term *arg = application->rTerm;
                string var = abstraction->var;
                Term *termToSub;

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
            break;
        case PRIMARY:
            if (term->rTerm != NULL) {
                changed |= betaReduce(term->rTerm);
            }
            break;
        case ABSTRACTION:
            changed |= betaReduce(term->lTerm);
            break;
    }

    return changed;
}

bool Parser::collapseParentheses(Term *term) {
    if (term == NULL) return false;
    bool changed = false;

    switch (term->type) {
        case APPLICATION:
            if (term->rTerm == NULL && term->lTerm->type == APPLICATION &&
                term->lTerm->rTerm == NULL) {
                Term *oldTerm = term->lTerm;
                term->lTerm = oldTerm->lTerm;
                delete oldTerm;

                changed = true;
            } else if (term->rTerm != NULL &&
                       term->lTerm->type == APPLICATION &&
                       term->lTerm->rTerm == NULL) {
                Term *oldTerm = term->lTerm;
                term->lTerm = oldTerm->lTerm;
                delete oldTerm;

                changed = true;
            } else if (term->lTerm->type == APPLICATION &&
                       term->lTerm->lTerm->type == PRIMARY) {
                Term *oldTerm = term->lTerm->lTerm;
                term->lTerm->type = oldTerm->type;
                term->lTerm->var = oldTerm->var;
                term->lTerm->lTerm = oldTerm->lTerm;
                delete oldTerm;

                changed = true;
            }

            changed |= collapseParentheses(term->lTerm);
            changed |= collapseParentheses(term->rTerm);
            break;
        case PRIMARY:
            changed |= collapseParentheses(term->rTerm);
            break;
        case ABSTRACTION:
            if (term->lTerm->type == APPLICATION &&
                term->lTerm->rTerm == NULL) {
                Term *oldTerm = term->lTerm->lTerm;
                term->lTerm->type = oldTerm->type;
                term->lTerm->var = oldTerm->var;
                term->lTerm->lTerm = oldTerm->lTerm;
                term->lTerm->rTerm = oldTerm->rTerm;
                delete oldTerm;

                changed = true;
            } else if (term->lTerm->type == APPLICATION &&
                       term->lTerm->lTerm->type == PRIMARY) {
                Term *oldTerm = term->lTerm->lTerm;
                term->lTerm->type = oldTerm->type;
                term->lTerm->var = oldTerm->var;
                term->lTerm->lTerm = oldTerm->lTerm;
                delete oldTerm;

                changed = true;
            }

            changed |= collapseParentheses(term->lTerm);
            break;
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

    switch (term->type) {
        case ABSTRACTION:
            return ("!" + term->var + "." + termToString(term->lTerm));
        case APPLICATION:
            return ("(" + termToString(term->lTerm) + ")" +
                    termToString(term->rTerm));
        case PRIMARY:
            string res = term->var;
            if (term->rTerm != NULL) res += " " + termToString(term->rTerm);
            return res;
            break;
    }

    runtimeError("Unable to convert term to string");
}

bool Parser::termToBool(Term *term) {
    if (term == NULL) runtimeError("Unable to convert term to bool");

    if (term->type == PRIMARY)
        runtimeError("A primary cannot be a bool");
    else if (term->type == APPLICATION) {
        if (term->lTerm->type != ABSTRACTION || term->rTerm != NULL)
            runtimeError("Unable to convert term to bool");

        term = term->lTerm;
    }

    if (term->lTerm->type != ABSTRACTION ||
        term->lTerm->lTerm->type != PRIMARY ||
        term->lTerm->lTerm->rTerm != NULL) {
        runtimeError("Unable to convert term to bool");
    }

    Term *outerAbs = term;
    Term *innerAbs = term->lTerm;
    Term *primary = term->lTerm->lTerm;
    string varA = outerAbs->var;
    string varB = innerAbs->var;
    string varC = primary->var;

    if (varA.compare(varC) == 0)
        return true;
    else if (varB.compare(varC) == 0)
        return false;

    runtimeError("Unable to convert term to bool");
}

int Parser::termToNum(Term *term) {
    if (term == NULL) runtimeError("Unable to convert term to number");

    if (term->type == PRIMARY)
        runtimeError("A primary cannot be a number");
    else if (term->type == APPLICATION) {
        if (term->lTerm->type != ABSTRACTION || term->rTerm != NULL)
            runtimeError("Unable to convert term to number");

        term = term->lTerm;
    }

    if (term->lTerm->type != ABSTRACTION || term->lTerm->lTerm->type != PRIMARY)
        runtimeError("Unable to convert term to number");

    int num = 0;
    Term *outerAbs = term;
    Term *innerAbs = outerAbs->lTerm;
    Term *primary = innerAbs->lTerm;
    term = primary;

    if (outerAbs->var.compare(innerAbs->var) == 0)
        runtimeError("Unable to convert term to number");

    while (term->var.compare(outerAbs->var) == 0) {
        num++;
        term = term->rTerm;

        if (term == NULL) runtimeError("Unable to convert term to number");

        switch (term->type) {
            case PRIMARY:
                if (term->var.compare(innerAbs->var) != 0)
                    runtimeError("Unable to convert term to number");
                break;
            case APPLICATION:
                if (term->rTerm != NULL)
                    runtimeError("Unable to convert term to number");

                term = term->lTerm;

                if (term->type != PRIMARY)
                    runtimeError("Unable to convert term to number");
                break;
            case ABSTRACTION:
                runtimeError("Unable to convert term to number");
                break;
        }
    }

    if (term->var.compare(innerAbs->var) != 0 || term->rTerm != NULL)
        runtimeError("Unable to convert term to number");

    return num;
}

Term *Parser::numToTerm(int num) {
    Term *outerAbs = new Term;
    Term *innerAbs = new Term;
    Term *basePrimary = new Term;

    outerAbs->type = ABSTRACTION;
    outerAbs->var = nextRenamedVar();
    outerAbs->lTerm = innerAbs;
    outerAbs->rTerm = NULL;

    innerAbs->type = ABSTRACTION;
    innerAbs->var = nextRenamedVar();
    innerAbs->lTerm = basePrimary;
    innerAbs->rTerm = NULL;

    basePrimary->type = PRIMARY;
    basePrimary->var = innerAbs->var;
    basePrimary->lTerm = NULL;
    basePrimary->rTerm = NULL;

    while (num > 0) {
        Term *term = new Term;
        term->type = APPLICATION;
        term->var = "";
        term->lTerm = innerAbs->lTerm;
        term->rTerm = NULL;

        Term *primary = new Term;
        primary->type = PRIMARY;
        primary->var = outerAbs->var;
        primary->lTerm = NULL;
        primary->rTerm = term;

        innerAbs->lTerm = primary;
        num--;
    }

    return outerAbs;
}