#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include <stdlib.h>
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;

int line = 1; // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code) {
    if (nTokens == MAX_TOKENS)err("too many tokens");
    Token *tk = &tokens[nTokens];
    tk->code = code;
    tk->line = line;
    nTokens++;
    return tk;
}

// copy in the dst buffer the string between [begin,end)
char *copyn(char *dst, const char *begin, const char *end) {
    char *p = dst;
    if (end - begin > MAX_STR)
        err("string too long");

    while (begin != end)
        *p++ = *begin++;
    *p = '\0';

    return dst;
}

void tokenize(const char *pch) {
    const char *start;
    Token *tk;
    char buf[MAX_STR + 1];
    for (;;) {
        switch (*pch) {
            case ' ':
            case '\t': pch++;
                break;

            case '\r': // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
                if (pch[1] == '\n')pch++;

            // fallthrough to \n
            case '\n':
                line++;
                pch++;
                break;

            case '\0':
                addTk(FINISH);
                return;

            case ',':
                addTk(COMMA);
                pch++;
                break;

            case '=':
                if (pch[1] == '=') {
                    addTk(EQUAL);
                    pch += 2;
                } else {
                    addTk(ASSIGN);
                    pch++;
                }
                break;

            case '/':
                if (pch[1] == '/') {
                    // addTk(COMMENT);
                    while (*pch != '\n' && *pch != '\0') pch++;
                }
                else {
                    addTk(DIV);
                    pch++;
                }
                break;

            case '+':
                addTk(ADD);
                pch++;
                break;

            case '-':
                addTk(SUB);
                pch++;
                break;

            case '*':
                addTk(MUL);
                pch++;
                break;

            case '(':
                addTk(LPAR);
                pch++;
                break;

            case ')':
                addTk(RPAR);
                pch++;
                break;

            case ':':
                addTk(COLON);
                pch++;
                break;

            case ';':
                addTk(SEMICOLON);
                pch++;
                break;

            case '!':
                if (pch[1] == '=') {
                    addTk(NOTEQ);
                    pch += 2;
                }
                else {
                    addTk(NOT);
                    pch++;
                }
                break;

            case '>':
                if (pch[1] == '=') {
                    addTk(GREATEREQ);
                    pch += 2;
                }
                else {
                    addTk(GREATER);
                    pch++;
                }
                break;

            case '<':
                addTk(LESS);
                pch++;
                break;

            case '&':
                if (pch[1] == '&') {
                    addTk(AND);
                    pch += 2;
                }
                else {
                    err("second & missing");
                }
                break;

            case '|':
                if (pch[1] == '|') {
                    addTk(OR);
                    pch += 2;
                }
                else {
                    err("second | missing");
                }
                break;

            default:
                if (isalpha(*pch) || *pch == '_') {
                    for (start = pch++; isalnum(*pch) || *pch == '_'; pch++) {}
                    char *text = copyn(buf, start, pch);

                    if (strcmp(text, "int") == 0) {
                        addTk(TYPE_INT);
                    }
                    else if (strcmp(text, "real") == 0) {
                        addTk(TYPE_REAL);
                    }
                    else if (strcmp(text, "str") == 0) {
                        addTk(TYPE_STR);
                    }

                    else if (strcmp(text, "end") == 0) {
                        addTk(END);
                    }

                    else if (strcmp(text, "if") == 0) {
                        addTk(IF);
                    }

                    else if (strcmp(text, "else") == 0) {
                        addTk(ELSE);
                    }

                    else if (strcmp(text, "while") == 0) {
                        addTk(WHILE);
                    }

                    else if (strcmp(text, "return") == 0) {
                        addTk(RETURN);
                    }

                    else if (strcmp(text, "var") == 0) {
                        addTk(VAR);
                    }

                    else if (strcmp(text, "function") == 0) {
                        addTk(FUNCTION);
                    }

                    else {
                        tk = addTk(ID);
                        strcpy(tk->text, text);
                    }
                }

                else if (isdigit(*pch) || *pch == '.') {
                    for (start = pch++; isdigit(*pch) || *pch == '.'; pch++) {}
                    char *text = copyn(buf, start, pch);
                    char *endptr;

                    if (strchr(text, '.')) {
                        if (!isdigit(text[strlen(text) - 1])) {
                            err("failed to convert to float: %s", text);
                        }
                        tk = addTk(REAL);
                        tk->r = strtod(text, &endptr);
                    }
                    else {
                        tk = addTk(INT);
                        tk->i = (int)strtol(text, &endptr, 10);
                        //
                        // if (*endptr != '\0') {
                        //     err("failed to convert to int: %s", endptr);
                        // }
                    }
                }

                else if (*pch == '"') {
                    for (start = ++pch; *pch != '"'; pch++) {
                        if (*pch == '\0') {
                            err("missing end \": line %d", line);
                        }
                    }

                    // if (*pch != '"') {
                    //     pch++;
                    //     char *text = copyn(buf, start, pch - 1);
                    //     err("missing end \": %s", text);
                    // }

                    pch++; // get rid of the trailing "
                    char *text = copyn(buf, start, pch - 1);

                    tk = addTk(STR);
                    strcpy(tk->text, text);
                }
                else
                {
                    showTokens();
                    err("invalid char: %c (%d)", *pch, *pch);
                }
        }
    }
}

void showTokens() {
    for (int i = 0; i < nTokens; i++) {
        Token *tk = &tokens[i];
        printf("%d\n", tk->code);
    }
}

const char *showTokenName(TokenType t) {
    switch (t) {
#define X(name) case name: return #name;
        TOKEN_LIST
#undef X
    }
    return "UNKNOWN";
}

void showNiceOutput() {
    for (int i = 0; i < nTokens; i++) {
        Token *tk = &tokens[i];

        printf("%d ", tk->line);

        if (tk->code == INT)
            printf("INT:%d",tk->i);

        else if (tk->code == REAL)
            printf("REAL:%f",tk->r);

        else if (tk->code == STR)
            printf("STR:%s",tk->text);

        else if (tk->code == ID)
            printf("ID:%s",tk->text);

        else
            printf("%s", showTokenName(tk->code));

        printf("\n");
    }
}