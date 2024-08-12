#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "incdir.h"

//structures and enums definitions

enum 
{
  	TEXTLEN = 500	//length of identifiers in input
};

//commands and default filenames
#define AOUT "a.out"
#ifdef __NASM__
#define ASCMD "nasm -f elf64 -w-ptr -pnasmext.inc -o "
#define LDCMD "cc -no-pie -fno-plt -Wall -o "
#else
#define ASCMD "as -o "
#define LDCMD "cc -o "
#endif
#define CPPCMD "cpp -nostdinc -isystem "

//token types
enum 
{
  	T_EOF,

  	// Binary operators
  	T_ASSIGN, T_ASPLUS, T_ASMINUS,
  	T_ASSTAR, T_ASSLASH, T_ASMOD,
	T_QUESTION, T_LOGOR, T_LOGAND,
  	T_OR, T_XOR, T_AMPER,
  	T_EQ, T_NE,
  	T_LT, T_GT, T_LE, T_GE,
 	T_LSHIFT, T_RSHIFT,
  	T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD,

  	//other operators
  	T_INC, T_DEC, T_INVERT, T_LOGNOT,

  	//type keywords
  	T_VOID, T_CHAR, T_INT, T_LONG,

  	//other keywords
  	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
  	T_STRUCT, T_UNION, T_ENUM, T_TYPEDEF,
  	T_EXTERN, T_BREAK, T_CONTINUE, T_SWITCH,
  	T_CASE, T_DEFAULT, T_SIZEOF, T_STATIC,

  	//structural tokens
  	T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
  	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
  	T_LBRACKET, T_RBRACKET, T_COMMA, T_DOT,
  	T_ARROW, T_COLON
};

//token structure
struct token 
{
  	int token;			//token type, from the enum list above
  	char *tokstr;			//string version of the token
  	int intvalue;			//for T_INTLIT, the integer value
};

// AST node types, the first few line up with the related tokens
enum 
{
  	A_ASSIGN = 1, A_ASPLUS, A_ASMINUS, A_ASSTAR,			//  1
  	A_ASSLASH, A_ASMOD, A_TERNARY, A_LOGOR,				//  5
  	A_LOGAND, A_OR, A_XOR, A_AND, A_EQ, A_NE, A_LT,			//  9
  	A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,				// 16
  	A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_MOD,			// 21
  	A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,				// 26
  	A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,			// 30
  	A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,				// 35
  	A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,			// 39
  	A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL, A_BREAK,		// 43
  	A_CONTINUE, A_SWITCH, A_CASE, A_DEFAULT, A_CAST			// 48
};

//primitive types. the bottom 4 bits is an integer value that represents the level of indirection,
// e.g. 0= no pointer, 1= pointer, 2= pointer pointer etc.
enum 
{
  	P_NONE, P_VOID = 16, P_CHAR = 32, P_INT = 48, P_LONG = 64,
  	P_STRUCT=80, P_UNION=96
};

//structural types
enum 
{
  	S_VARIABLE, S_FUNCTION, S_ARRAY
};

//storage classes
enum 
{
  	C_GLOBAL = 1,			//globally visible symbol
  	C_LOCAL,			//locally visible symbol
  	C_PARAM,			//locally visible function parameter
  	C_EXTERN,			//external globally visible symbol
  	C_STATIC,			//static symbol, visible in one file
  	C_STRUCT,			//struct
  	C_UNION,			//union
  	C_MEMBER,			//member of struct or union
  	C_ENUMTYPE,			//named enumeration type
  	C_ENUMVAL,			//named enumeration value
  	C_TYPEDEF			//named typedef
};

//symbol table structure
typedef struct symtable 
{
  	char *name;			//name of a symbol
  	int type;			//primitive type for the symbol
  	struct symtable *ctype;		//if struct/union, ptr to that type
  	int stype;			//structural type for the symbol
  	int class;			//storage class for the symbol
  	int size;			//total size in bytes of this symbol
  	int nelems;			//functions: # params. Arrays: # elements
#define st_endlabel st_posn		//for functions, the end label
  	int st_posn;			//for locals, the negative offset
    					//from the stack base pointer
  	int *initlist;			//list of initial values
  	struct symtable *next;		//next symbol in one list
  	struct symtable *member;	//first member of a function, struct,
} symt;					//union or enum

// Abstract Syntax Tree structure
typedef struct ASTnode 
{
  	int op;				// "Operation" to be performed on this tree
  	int type;			//type of any expression this tree generates
  	struct symtable *ctype;		//if struct/union, ptr to that type
  	int rvalue;			//true if the node is an rvalue
  	struct ASTnode *left;		//left, middle and right child trees
  	struct ASTnode *mid;
  	struct ASTnode *right;
  	struct symtable *sym;		//for many AST nodes, the pointer to
  					//the symbol in the symbol table
#define a_intvalue a_size		//for A_INTLIT, the integer value
  	int a_size;			//for A_SCALE, the size to scale by
} ast;

enum 
{
  	NOREG = -1,			//use NOREG when the AST generation
  					//functions have no register to return
  	NOLABEL = 0			//use NOLABEL when we have no label to pass to genAST()
};
