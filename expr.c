#include "defs.h"
#include "data.h"
#include "decl.h"

//parsing of expressions

//parse a primary factor and return an AST node..
static ast *primary(void) 
{
  	ast *n;
  	int id;

  	switch(Token.token) 
  	{
    		case T_INTLIT:
      			//for an INTLIT token, make a leaf AST node for it.
      			n = mkastleaf(A_INTLIT, Token.intvalue);
      			break;

    		case T_IDENT:
      			//check that this ident exists or not if it is then it returns index num else -1..
      			id = findglob(Text);
      			if(id == -1)
				fatals("Unknown variable", Text);

		        //make a leaf AST node for it
      			n = mkastleaf(A_IDENT, id);
      			break;

    		default:
      			fatald("Syntax error, token", Token.token);
  	}

  	//scan in the next token and return the leaf node
  	scan(&Token);
  	return n;
}


//convert a binary operator token into an AST operation. we rely on a 1:1 mapping from token to AST operation
static int arithop(int tokentype) 
{
  	if (tokentype > T_EOF && tokentype < T_INTLIT)
    		return (tokentype);
  	fatald("Syntax error, token", tokentype);
}

//operator precedence for each token, must match up with the order of tokens in defs.h
static int OpPrec[] = 
{
  	0, 10, 10,		// T_EOF, T_PLUS, T_MINUS
  	20, 20,			// T_STAR, T_SLASH
  	30, 30,			// T_EQ, T_NE
  	40, 40, 40, 40		// T_LT, T_GT, T_LE, T_GE
};

//check that we have a binary operator and return its precedence otherwise return with error..
static int op_precedence(int tokentype) 
{
  	int prec = OpPrec[tokentype];
  	if(prec == 0)
    		fatald("Syntax error, token", tokentype);
  	return prec;
}

//return an AST tree whose root is a binary operator. parameter ptp is the previous token's precedence.
ast *binexpr(int ptp) 
{
  	ast *left, *right;
  	int tokentype;

  	//get the primary tree on the left.
  	//fetch the next token at the same time.
  	left = primary();

  	//if we hit a semicolon or ')', return just the left node
  	tokentype = Token.token;
  	if(tokentype == T_SEMI || tokentype == T_RPAREN)
    		return (left);

  	//while the precedence of this token is more than that of the previous token precedence
  	while (op_precedence(tokentype) > ptp) 
  	{
    		//fetch in the next integer literal
    		scan(&Token);

    		//recursively call binexpr() with the precedence of our token to build a sub-tree
    		right = binexpr(OpPrec[tokentype]);

    		//join that sub-tree with ours. convert the token into an AST operation at the same time.
    		left = mkastnode(arithop(tokentype), left, NULL, right, 0);

    		//update the details of the current token.
    		//if we hit a semicolon or ')', return just the left node
    		tokentype = Token.token;
    		if(tokentype == T_SEMI || tokentype == T_RPAREN)
      			return left;
  	}

  	//return the tree we have when the precedence is the same or lower
  	return left;
}
