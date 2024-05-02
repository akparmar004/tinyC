#include"data.h"
#include"defs.h"
#include"decl.h"

ast *primary(void)
{
		ast *n;

		switch(Token.token)
		{
			case T_INTLIT:
			        n = mkastleaf(A_INTLIT,Token.intvalue);
		       		scan(&Token);
			 	return n;
			default:
				fprintf(stderr, "syntax error on line %d, token %d\n", Line, Token.token);
    				exit(1);	
		}
}

//convert a binary operator token into an AST operation.
static int arithop(int tok) 
{
	switch (tok) 
	{
  		case T_PLUS:
    			return (A_ADD);
  		case T_MINUS:
    			return (A_SUBTRACT);
  		case T_STAR:
    			return (A_MULTIPLY);
  		case T_SLASH:
    			return (A_DIVIDE);
  		default:
    			fprintf(stderr, "syntax error on line %d, token %d\n", Line, tok);
    			exit(1);
  	}
}

ast* additive_expr(void);


ast* multiplicative_expr(void)
{
	ast *left, *right;
	int tokentype;

	//get the int lit at the left..
	left = primary();

	//if no tokens left return just the left node..
	tokentype = Token.token;
	if(tokentype == T_EOF)
		return left;

	//while the token = '*' | '/'
	while((tokentype == T_STAR) || (tokentype == T_SLASH))
	{
		//fetch the next int lit..
		scan(&Token);
		right = primary();

		//join that with the left int lit..
		left = mkastnode(arithop(tokentype),left,right,0);

		//update the details of the current token..
		tokentype = Token.token;
		if(tokentype == T_EOF)
			break;
	}
	//return the tree that we created..
	return left;

}


ast* additive_expr(void)
{
	ast *left, *right;
	int tokentype;

	//get the left subtree at a higher precedence than us
	left = multiplicative_expr();

	//if no tokens left then simply return just left node..
	tokentype = Token.token;
	if(tokentype == T_EOF)
		return left;

	//run a loop that works on our level of precedence
	while(1)
	{
		scan(&Token);	//fetch in the next int literal

		//get the right subtree
		right = multiplicative_expr();

		//join both sub trees with our low precedence operator...
		left = mkastnode(arithop(tokentype),left,right,0);

		tokentype = Token.token;
		if(tokentype == T_EOF)	
			break;
	}
	//return tree that we have created
	return left;
}

ast* binexpr(int n)
{
	return additive_expr();
}

