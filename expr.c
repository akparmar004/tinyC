#include "defs.h"
#include "data.h"
#include "decl.h"

static ast *primary(void)
{
	ast *n;
	
	switch(Token.token)
	{
		case T_INTLIT :
			n = mkastleaf(A_INTLIT,Token.intvalue);
			scan(&Token);
			return n;
		default : 
			fprintf(stderr,"syntax error on line %d\n",Line);
			exit(1);
	}
} 

int arithop(int tok)
{
	switch(tok)
	{
		case T_PLUS :
			return A_ADD;
		case T_MINUS :
			return A_SUBTRACT;
		case T_STAR :
			return A_MULTIPLY;
		case T_SLASH :
			return A_DIVIDE;
		case default:
			fprintf(stderr, "unknown token in arithop() on line %d\n",Line);
			exit(1);	 
	}
}

ast *binexpr(void)
{
	ast *n, *left, *right;
	int nodetype;
	
	//fetch the token
	left = primary();
	
	if(Token.token == T_EOF)
		return left;
		
	nodetype = arithop(Token.token);
	
	//get the next token..
	scan(&Token);
	
	//recursively get the right hand tree..	
	right = binexpr();
	
	//now build a tree with both sub-trees..
	n = mkastnode(nodetype,left,right,0);
	
	return n;
}
