#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


struct token
{
	int token;
	int intvalue;
};

enum
{
	T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT
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
