#include "defs.h"
#include "data.h"
#include "decl.h"

static ast* primary(void)
{
	ast *n;
	int id;	
	switch(Token.token)
	{
		case T_INTLIT :
			n = mkastleaf(A_INTLIT,Token.intvalue);
			break;
		case T_IDENT :
			//check that this iden.. exists or not..
			id = findglob(Text);
			if(id == -1)
				fatals("Unknown variable", Text);
				
			//make a leafnode
			n = mkastleaf(A_IDENT,id);
			break;
		default : 
			fatald("Syntax error, token", Token.token);
	}
	scan(&Token);
	return n;
} 

int arithop(int tokentype)
{
	switch(tokentype)
	{
		case T_PLUS :
			return A_ADD;
		case T_MINUS :
			return A_SUBTRACT;
		case T_STAR :
			return A_MULTIPLY;
		case T_SLASH :
			return A_DIVIDE;
		default:
			fatald("Syntax error, token", tokentype);		
	}
}

//operatoc precedence for each token
static int OpPrec[] = {0, 10, 10, 20, 20, 0};

//check for binary ope. and return its precedence..
static int op_precedence(int tokentype)
{
	int prec = OpPrec[tokentype];
	if(prec == 0)
	{
		fatald("Syntax error, token", tokentype);
	}
	return prec;
}


ast *binexpr(int ptp)
{
	ast *left, *right;
	int tokentype;
	
	//fetch the token on the left
	left = primary();
	
	tokentype = Token.token;
	if(tokentype == T_SEMI)
		return left;
	
	//while the prece. of this token is more than that of the previous token prec.
	while(op_precedence(tokentype) > ptp)	
	{
		//fetch in the next int lit..
		scan(&Token);

		//recursively call the binexpr() with the prece. of our token to build a sub tree..
		right = binexpr(OpPrec[tokentype]);
		
		//simply join both subtrees 
		left = mkastnode(arithop(tokentype), left, right, 0);

		//now update the details of the current token..
		tokentype = Token.token;
		if(tokentype == T_SEMI)
			return left;
	}
	
	//just return the tree that we have..
	return left;
}
