#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

enum
{
	T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT
};


struct token
{
	int token;
	int intvalue;
};

//AST node types..
enum
{
	A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_INTLIT
};

//AST structure
typedef struct ASTnode
{
	int op;
	struct ASTnode *left;
	struct ASTnode *right;
	int intvalue;
} ast;
