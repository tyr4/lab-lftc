#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ad.h"
#include "lexer.h"

int iTk; // the iterator in tokens
Token *consumed; // the last consumed token

bool block();
bool expr();

// same as err, but also prints the line of the current token
_Noreturn void tkerr(const char *fmt, ...) {
    fprintf(stderr, "error in line %d: ", tokens[iTk].line);
    va_list va;

    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    fprintf(stderr, "\n");

    delDomain();
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    // printf("about to consume %s\n", showTokenName(tokens[iTk].code));
    if (tokens[iTk].code == code) {
        // printf("CONSUMED %s %d\n", showTokenName(tokens[iTk].code), tokens[iTk].line);

        consumed = &tokens[iTk++];

        return true;
    }

    // printf("FAILED to consume %s\n", showTokenName(tokens[iTk].code));
    return false;
}

bool factor() {
    int start = iTk;

    if (consume(INT)) {
        return true;
    }
    if (consume(REAL)) {
        return true;
    }
    if (consume(STR)) {
        return true;
    }

    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            } else tkerr("missing ) after expression");
        } else tkerr("invalid expression format");
    }

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                for (;;) {
                    if (consume(COMMA)) {
                        if (expr()) {
                            continue;
                        } else tkerr("missing expression after ,");
                    }
                    break;
                }
            }

            if (consume(RPAR)) {}
            else tkerr("missing ) after expression");
        }

        return true;
    }

    iTk = start;
    return false;
}

bool exprPrefix() {
    int start = iTk;

    if (consume(SUB)) {}
    else if (consume(NOT)) {}

    if (factor()) {
        return true;
    }

    iTk = start;
    return false;
}

bool exprMul() {
    int start = iTk;

    if (exprPrefix()) {
        for (;;) {
            if (consume(MUL)) {
                if (exprPrefix()) {
                    continue;
                } else tkerr("invalid expression after multiplication");
            }

            else if (consume(DIV)) {
                if (exprPrefix()) {
                    continue;
                } else tkerr("invalid expression after division");
            }

            else break;
        }

        return true;
    }

    iTk = start;
    return false;
}

bool exprAdd() {
    int start = iTk;

    if (exprMul()) {
        for (;;) {
            if (consume(ADD)) {
                if (exprMul()) { }
                else tkerr("invalid expression after addition");
            }

            else if (consume(SUB)) {
                if (exprMul()) { }
                else tkerr("invalid expression after subtraction");
            }

            else break;
        }

        return true;
    }

    iTk = start;
    return false;
}

bool exprComp() {
    int start = iTk;

    if (exprAdd()) {
        if (consume(LESS)) {
            if (exprAdd()) {}
            else tkerr("invalid expression after <");
        }

        if (consume(EQUAL)) {
            if (exprAdd()) {}
            else tkerr("invalid expression after ==");
        }

        if (consume(GREATER)) {
            if (exprAdd()) {}
            else tkerr("invalid expression after >");
        }

        if (consume(GREATEREQ)) {
            if (exprAdd()) {}
            else tkerr("invalid expression after >=");
        }

        return true;
    }

    iTk = start;
    return false;
}

bool exprAssign() {
    int start = iTk;

    if (consume(ID)) {
        if (consume(ASSIGN)) {
            if (exprComp()) return true;
            else tkerr("invalid expression after =");
        }

        iTk = start;
    }

    return exprComp();
}

bool exprLogic() {
    int start = iTk;

    if (exprAssign()) {
        for (;;) {
            int save = iTk;

            if (consume(AND)) {
                if (exprAssign()) {
                    continue;
                } else tkerr("invalid expression after &&");
            }
            else if (consume(OR)) {
                if (exprAssign()) {
                    continue;
                } else tkerr("invalid expression after ||");
            }

            iTk = save;
            break;
        }

        return true;
    }

    iTk = start;
    return false;
}

bool expr() {
    int start = iTk;

    if (exprLogic()) {
        return true;
    }

    iTk = start;
    return false;
}

bool instr() {
    int start = iTk;

    if (expr()) {}
    if (consume(SEMICOLON)) {
        return true;
    }

    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {

                        if (consume(ELSE)) {
                            if (block()) {}
                            else tkerr("invalid block format inside 'else'");
                        }

                        if (consume(END)) {
                            return true;
                        } else tkerr("missing 'end' keyword after if/else statement");
                    } else tkerr("invalid block format inside 'if'");
                } else tkerr("missing ) after '%s' inside expression", strcmp(consumed->text, "")? consumed->text : consumedName(consumed));
            } else tkerr("invalid expression format inside 'if'");
        } else tkerr("missing ( after 'if'");
    }

    if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            } else tkerr("missing ; after expression inside 'return' statement");
        } else tkerr("invalid expression format after 'return' statement");
    }

    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        } else tkerr("missing 'end' keyword after 'while' statement");
                    } else tkerr("invalid block format inside 'while' statement");
                } else tkerr("missing ) after expression inside 'while' statement");
            } else tkerr("invalid expression format inside 'while' statement");
        } else tkerr("missing ( after 'while' statement");
    }

    iTk = start;
    return false;
}

bool block() {
    int start = iTk;

    if (instr()) {
        for (;;) {
            if (instr()) {}
            else break;
        }

        return true;
    }

    iTk = start;
    return false;
}

bool baseType() {
    if (consume(TYPE_INT)) {
        ret.type = TYPE_INT;

        return true;
    }
    if (consume(TYPE_REAL)) {
        ret.type = TYPE_REAL;

        return true;
    }
    if (consume(TYPE_STR)) {
        ret.type = TYPE_STR;

        return true;
    }

    return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    int start = iTk;

    if (consume(VAR)) {
        if (consume(ID)) {

            const char *name = consumed->text;
            Symbol *s = searchInCurrentDomain(name);
            if (s) tkerr("symbol redefinition: %s", name);

            s = addSymbol(name, KIND_VAR);
            s->local = crtFn != NULL;

            if (consume(COLON)) {
                if (baseType()) {
                    s->type = ret.type;

                    if (consume(SEMICOLON)) {
                        return true;
                    } else tkerr("missing ;");
                } else tkerr("incorrect base type: %s", tokens[iTk].text);
            } else tkerr("missing : after '%s'", tokens[iTk].text);
        } else tkerr("missing identificator");
    }

    iTk = start;
    return false;
}

bool funcParam() {
    int start = iTk;

    if (consume(ID)) {
        const char *name = consumed->text;
        Symbol *s = searchInCurrentDomain(name);
        if (s) tkerr("symbol redefinition: %s", name);

        s = addSymbol(name,KIND_ARG);
        Symbol *sFnParam=addFnArg(crtFn,name);

        if (consume(COLON)) {
            if (baseType()) {
                s->type = ret.type;
                sFnParam->type = ret.type;

                return true;
            } else tkerr("incorrect base type: %s", tokens[iTk].text);
        } else tkerr("missing : after '%s'", consumed->text);
    }

    iTk = start;
    return false;
}

bool funcParams() {
    int start = iTk;

    if (funcParam()) {
        for (;;) {
            int save = iTk;

            if (consume(COMMA)) {
                if (funcParam()) {
                    continue;
                }
            }

            iTk = save;
            break;
        }

        return true;
    }

    iTk = start;
    return false;
}

bool defFunc() {
    int start = iTk;

    if (consume(FUNCTION)) {
        if (consume(ID)) {

            const char *name = consumed->text;
            Symbol *s = searchInCurrentDomain(name);
            if (s) tkerr("symbol redefinition: %s", name);

            crtFn = addSymbol(name, KIND_FN);
            crtFn->args = NULL;
            addDomain();

            if (consume(LPAR)) {
                if (funcParams()) {}

                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            crtFn->type = ret.type;

                            for (;;) {
                                if (defVar()) {}
                                else break;
                            }
                            if (block()) {
                                if (consume(END)) {
                                    delDomain();
                                    crtFn = NULL;

                                    return true;
                                } else tkerr("missing 'end' keyword inside function block");
                            } else tkerr("incorrect function body");
                        } else tkerr("incorrect function return type");
                    } else tkerr("missing : after ) inside function definition");
                } else tkerr("missing ) after function definition");
            } else tkerr("missing ( after function name");
        } else tkerr("missing name after function definition");
    }

    iTk = start;
    return false;
}

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    addDomain();

    for (;;) {
        if (defVar()) {}
        else if (defFunc()) {}
        else if (block()) {}

        else break;
    }

    if (consume(FINISH)) {
        delDomain();

        printf("\nsintaxa ok!\n");
        return true;
    }

    else tkerr("syntax error");

    return false;
}

void parse() {
    iTk = 0;
    program();
}