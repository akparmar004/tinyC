#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//structure and enum definitions

#define TEXTLEN		512	//maximum length of particular symbol
#define NSYMBOLS        1024	//maximum number of symbol table entries 

enum 
{	//Token types
  	T_EOF,
  	T_PLUS, T_MINUS,
  	T_STAR, T_SLASH,
  	T_EQ, T_NE,
  	T_LT, T_GT, T_LE, T_GE,
  	T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,
  	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
 	
	//tokens for keywords
  	T_PRINT, T_INT, T_IF, T_ELSE, T_WHILE,
	T_FOR, T_VOID
};

struct token
{ 			
	//Token structure
  	int token;			//token type, from the enum list above
  	int intvalue;			//for T_INTLIT, the integer value
};

//AST node types..
enum 
{
  	A_ADD = 1, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
  	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
  	A_INTLIT,
  	A_IDENT, A_LVIDENT, A_ASSIGN, A_PRINT, A_GLUE,
  	A_IF, A_WHILE, A_FUNCTION
};

//Abstract Syntax Tree(AST) structure
typedef struct ASTnode 
{
  	int op;				//"Operation" to be performed on this tree
  	struct ASTnode *left;		//left, middle and right child trees
  	struct ASTnode *mid;
  	struct ASTnode *right;
  	union 
	{
    		int intvalue;		//for A_INTLIT, the integer value
    		int id;			//for A_IDENT, the symbol slot number
  	} v;
}ast;
#define NOREG	-1	    //use NOREG when the AST generation functions have no register available to return

//symbol table structure
struct symtable 
{
  	char *name;	//symbol name..
};
