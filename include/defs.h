#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "incdir.h"

//structures and enums definitions

enum 
{
  	TEXTLEN = 500	
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

  	T_VOID, T_CHAR, T_INT, T_LONG,

  	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
  	T_STRUCT, T_UNION, T_ENUM, T_TYPEDEF,
  	T_EXTERN, T_BREAK, T_CONTINUE, T_SWITCH,
  	T_CASE, T_DEFAULT, T_SIZEOF, T_STATIC,

  	T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
  	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
  	T_LBRACKET, T_RBRACKET, T_COMMA, T_DOT,
  	T_ARROW, T_COLON
};

struct token 
{
  	int token;			
  	char *tokstr;			
  	int intvalue;			
};

enum 
{
  	A_ASSIGN = 1, A_ASPLUS, A_ASMINUS, A_ASSTAR,			
  	A_ASSLASH, A_ASMOD, A_TERNARY, A_LOGOR,				
  	A_LOGAND, A_OR, A_XOR, A_AND, A_EQ, A_NE, A_LT,		
  	A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,			
  	A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_MOD,			
  	A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,				
  	A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,			
  	A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,				
  	A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,			
  	A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL, A_BREAK,		
  	A_CONTINUE, A_SWITCH, A_CASE, A_DEFAULT, A_CAST			
};

enum 
{
  	P_NONE, P_VOID = 16, P_CHAR = 32, P_INT = 48, P_LONG = 64,
  	P_STRUCT=80, P_UNION=96
};

enum 
{
  	S_VARIABLE, S_FUNCTION, S_ARRAY
};

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

typedef struct symtable 
{
  	char *name;			
  	int type;			
  	struct symtable *ctype;		
  	int stype;			
  	int class;			
  	int size;			
  	int nelems;			
#define st_endlabel st_posn		
  	int st_posn;			
    					
  	int *initlist;			
  	struct symtable *next;		
  	struct symtable *member;	
} symt;					

typedef struct ASTnode 
{
  	int op;				
  	int type;			
  	struct symtable *ctype;		
  	int rvalue;			
  	struct ASTnode *left;		
  	struct ASTnode *mid;
  	struct ASTnode *right;
  	struct symtable *sym;		
  					
#define a_intvalue a_size		
  	int a_size;			
} ast;

enum 
{
  	NOREG = -1,			
  					
  	NOLABEL = 0			
};
