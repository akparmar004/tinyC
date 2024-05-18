#include "defs.h"
#include "data.h"
#include "decl.h"

//miscellaneous functions

//check that the current token is t, and fetch next token. else throw an error 
void match(int tok, char *what) 
{
  	if(Token.token == tok) 
	{
    		scan(&Token);
  	}
       	else 
	{
    		fatals("Expected", what);
  	}
}

//check a semicolon and fetch next token
void semi(void) 
{
  	match(T_SEMI, ";");
}

//check right brackate and fetch next token
void rbrace(void) 
{
  	match(T_RBRACE, "}");
}

//check left brackate and fetch next token
void lbrace(void) 
{
  	match(T_LBRACE, "{");
}

//check left parenthesis and fetch next token
void lparen(void) 
{
  	match(T_LPAREN, "(");
}

//match  right parenthesis and fetch the next token
void rparen(void) 
{
  	match(T_RPAREN, ")");
}

//match identifier and fetch next token
void ident(void) 
{
  	match(T_IDENT, "identifier");
}

//print fatal messages
void fatal(char *s) 
{
  	fprintf(stderr, "%s on line %d\n", s, Line);
  	exit(1);
}

void fatals(char *s1, char *s2) 
{
  	fprintf(stderr, "%s:%s on line %d\n", s1, s2, Line);
  	exit(1);
}

void fatald(char *s, int d) 
{
  	fprintf(stderr, "%s:%d on line %d\n", s, d, Line);
  	exit(1);
}

void fatalc(char *s, int c) 
{
  	fprintf(stderr, "%s:%c on line %d\n", s, c, Line);
  	exit(1);
}
