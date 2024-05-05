#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//structure and enum definitions

#define TEXTLEN		512	//length of symbols in input
#define NSYMBOLS        1024	//number of symbol table entries

//types of tokens
enum 
{
  	T_EOF, 
  	T_PLUS, T_MINUS, T_STAR, T_SLASH,
  	T_EQ, T_NE,
  	T_LT, T_GT, 
  	T_LE, T_GE,
  	T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,
  
  	//keywords
  	T_PRINT, T_INT
};

//token structure
struct token 
{
  	int token;			//token type, from the enum list above
  	int intvalue;			//integer value for intlit token
};

//AST node types
enum 
{
  	A_ADD = 1, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,       
	A_INTLIT,
  	A_IDENT, A_LVIDENT, A_ASSIGN
};

//abstract Syntax Tree structure
typedef struct ASTnode 
{
  	int op;			// "Operation" to be performed on this tree
  	struct ASTnode *left;		// Left and right child trees
  	struct ASTnode *right;
  	union 
	{
    		int intvalue;		//for A_INTLIT, the integer value
    		int id;			//for A_IDENT, the symbol slot number
  	} v;
}ast;

//symbol table structure
struct symtable
{
  	char *name;			//name of a symbol
};
