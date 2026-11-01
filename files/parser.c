#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    // printf("about to consume %s\n", showTokenName(tokens[iTk].code));
    if (tokens[iTk].code == code) {
        printf("CONSUMED %s %d\n", showTokenName(tokens[iTk].code), tokens[iTk].line);

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
        int save = iTk;

        if (consume(LPAR)) {
            if (expr()) {
                for (;;) {
                    int save2 = iTk;

                    if (consume(COMMA)) {
                        if (expr()) {
                            continue;
                        }
                    }

                    iTk = save2;
                    break;
                }
            }
            else iTk = save;

            if (consume(RPAR)) {}
            else iTk = save;
        }
        else iTk = save;

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
            int save = iTk;

            if (consume(MUL) || consume(DIV)) {
                if (exprPrefix()) {
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

bool exprAdd() {
    int start = iTk;

    if (exprMul()) {
        for (;;) {
            int save = iTk;

            if (consume(ADD) || consume(SUB)) {
                if (exprMul()) {
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

bool exprComp() {
    int start = iTk;

    if (exprAdd()) {
        if (consume(LESS) || consume(EQUAL) || consume(GREATER) || consume(GREATEREQ)) {
            if (exprAdd()) {}
        }

        return true;
    }

    iTk = start;
    return false;
}

bool exprAssign() {
    int start = iTk;

    if (consume(ID)) {
        if (consume(ASSIGN)) {}
        else iTk = start;
    }
    else iTk = start;

    if (exprComp()) {
        return true;
    }

    iTk = start;
    return false;
}

bool exprLogic() {
    int start = iTk;

    if (exprAssign()) {
        for (;;) {
            int save = iTk;

            if (consume(AND) || consume(OR)) {
                if (exprAssign()) {
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
                        int save = iTk;

                        if (consume(ELSE)) {
                            if (block()) {}
                            else tkerr("invalid block format");
                        }
                        else iTk = save;

                        if (consume(END)) {
                            return true;
                        } else tkerr("missing 'end' keyword");
                    } else tkerr("invalid block format");
                } else tkerr("missing ) after '%s'", strcmp(consumed->text, "")? consumed->text : consumedName(consumed));
            } else tkerr("invalid expression format");
        } else tkerr("missing ( after 'if'");
    }

    if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            } else tkerr("missing ;");
        } else tkerr("invalid expression format");
    }

    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        } else tkerr("missing 'end' keyword");
                    } else tkerr("invalid block format");
                } else tkerr("missing ) after expression");
            } else tkerr("invalid expression format");
        } else tkerr("missing ( after 'while'");
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
        return true;
    }
    if (consume(TYPE_REAL)) {
        return true;
    }
    if (consume(TYPE_STR)) {
        return true;
    }

    return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    int start = iTk;

    if (consume(VAR)) {
        if (consume(ID)) {
            if (consume(COLON)) {
                if (baseType()) {
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
        if (consume(COLON)) {
            if (baseType()) {
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
            if (consume(LPAR)) {
                if (funcParams()) {}

                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            for (;;) {
                                if (defVar()) {}
                                else break;
                            }
                            if (block()) {
                                if (consume(END)) {
                                    return true;
                                } else tkerr("missing 'end' keyword");
                            } else tkerr("incorrect function body");
                        } else tkerr("incorrect base type");
                    } else tkerr("missing :");
                } else tkerr("missing )");
            } else tkerr("missing (");
        } else tkerr("missing ID");
    }

    iTk = start;
    return false;
}

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    for (;;) {
        if (defVar()) {}
        else if (defFunc()) {}
        else if (block()) {}

        else break;
    }

    if (consume(FINISH)) {
        return true;
    }

    else tkerr("syntax error");

    return false;
}

void parse() {
    iTk = 0;
    program();
}