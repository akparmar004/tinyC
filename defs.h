#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//enums and structures..

#define TEXTLEN		512	//length of symbols in input
#define NSYMBOLS        1024	//total number of symbol can be define..

//token types
enum
{
  	T_EOF,
  	
	//operators
  	T_ASSIGN, T_PLUS, T_MINUS, T_STAR, T_SLASH,
  	T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,
  
	//type keywords
  	T_VOID, T_CHAR, T_INT, T_LONG,
  
	//structural tokens
  	T_INTLIT, T_SEMI, T_IDENT, T_LBRACE, T_RBRACE, T_LPAREN, 
	T_RPAREN, T_LBRACKET, T_RBRACKET, T_AMPER, T_LOGAND,
  	
	//other keywords
  	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN
};

//token structure
struct token 
{
  	int token;			//token type, from the enum list above
  	int intvalue;			//for T_INTLIT, the integer value
};


//AST node types, The first few line up with the related tokens
enum 
{
  	A_ASSIGN= 1, A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
  	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
  
	A_INTLIT, A_IDENT, A_GLUE,
  	
	A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,
  	
	A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE
};

//primitive types
enum 
{
  	P_NONE, P_VOID, P_CHAR, P_INT, P_LONG,
  	P_VOIDPTR, P_CHARPTR, P_INTPTR, P_LONGPTR
};

//Abstract Syntax Tree structure
typedef struct ASTnode 
{
  	int op;				//"Operation" to be performed on this tree
  	int type;			//type of any expression this tree generates
  	int rvalue;			//true if the node is an rvalue
  	struct ASTnode *left;		//left tree 
  	struct ASTnode *mid;		//mid tree
  	struct ASTnode *right;		//right tree..
  	union 
	{				//for A_INTLIT, the integer value
    		int intvalue;		//for A_IDENT, the symbol slot number
    		int id;			//for A_FUNCTION, the symbol slot number
    		int size;		//for A_SCALE, the size to scale by
  	} v;				//for A_FUNCCALL, the symbol slot number
} ast;

#define NOREG	-1		//use NOREG when the AST generation functions have no register to return
#define NOLABEL	 0		//use NOLABEL when we have no label to pass to genAST()

//structural types 
enum 
{
  	S_VARIABLE, S_FUNCTION, S_ARRAY
};

//symbol table structure
typedef struct symtable 
{
  	char *name;			//name of symbol
  	int type;			//primitive type 
  	int stype;			//structural type 
  	int endlabel;			//for S_FUNCTIONs, the end label
  	int size;			//number of elements in the symbol
} symt;
