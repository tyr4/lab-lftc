#pragma once

#define TOKEN_LIST \
/* id */ \
X(ID) \
/* keywords */ \
X(VAR) X(FUNCTION) X(IF) X(ELSE) X(WHILE) X(END) X(RETURN) \
X(TYPE_INT) X(TYPE_REAL) X(TYPE_STR) \
/* delimiters */ \
X(COMMA) X(COLON) X(SEMICOLON) X(LPAR) X(RPAR) X(FINISH) \
/* operators */ \
X(ADD) X(SUB) X(MUL) X(DIV) X(AND) X(OR) X(NOT) X(ASSIGN) \
X(EQUAL) X(NOTEQ) X(LESS) X(GREATER) X(GREATEREQ) \
/* extras */ \
X(SPACE) X(COMMENT) \
/* constants */ \
X(INT) X(REAL) X(STR)

typedef enum {
#define X(name) name,
	TOKEN_LIST
#undef X
} TokenType;

// enum {
// 	ID
// 	// keywords
// 	,VAR, FUNCTION, IF, ELSE, WHILE, END, RETURN, TYPE_INT, TYPE_REAL, TYPE_STR
// 	// delimiters
// 	,COMMA, COLON, SEMICOLON, LPAR, RPAR, FINISH
// 	// operators
// 	,ADD, SUB, MUmi a L, DIV, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, GREATER, GREATEREQ
// 	// extras
// 	,SPACE, COMMENT
// 	// constants
// 	,INT, REAL, STR
// };

#define MAX_STR		127

typedef struct{
	int code;		// ID, TYPE_INT, ...
	int line;		// the line from the input file
	union{
		char text[MAX_STR+1];		// the chars for ID, STR
		int i;		// the value for INT
		double r;		// the value for REAL
		};
	}Token;

#define MAX_TOKENS		4096
extern Token tokens[];
extern int nTokens;

void tokenize(const char *pch);
void showTokens();

const char *token_name(TokenType t);
const char *showTokenName(TokenType t);
void showNiceOutput();