#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//AST tree Optimisation Code

static ast *fold2(ast *n) 
{
  	int val, leftval, rightval;

  	leftval = n->left->a_intvalue;
  	rightval = n->right->a_intvalue;

  	switch(n->op) 
  	{
    		case A_ADD:
      			val = leftval + rightval;
      			break;
    		case A_SUBTRACT:
      			val = leftval - rightval;
      			break;
    		case A_MULTIPLY:
      			val = leftval * rightval;
      			break;
    		case A_DIVIDE:
      			if (rightval == 0)
				return (n);
      			val = leftval / rightval;
      			break;
    		default:
      			return (n);
  		}

  	return (mkastleaf(A_INTLIT, n->type, NULL, NULL, val));
}

static ast *fold1(ast *n) 
{
  	int val;

  	val = n->left->a_intvalue;
  	switch (n->op) 
  	{
    		case A_WIDEN:
      			break;
    		case A_INVERT:
      			val = ~val;
      			break;
    		case A_LOGNOT:
      			val = !val;
      			break;
    		default:
      			return (n);
  	}

  	return (mkastleaf(A_INTLIT, n->type, NULL, NULL, val));
}

static ast *fold(ast *n) 
{
  	if(n == NULL)
    		return (NULL);

  	n->left = fold(n->left);
  	n->right = fold(n->right);

  	if(n->left && n->left->op == A_INTLIT) 
  	{
    		if (n->right && n->right->op == A_INTLIT)
      			n = fold2(n);
    		else
     			// If only the left is A_INTLIT, do a fold1()
      			n = fold1(n);
  	}
  	return n;
}

ast *optimise(ast *n)
{
  	n = fold(n);
  	return n;
}
