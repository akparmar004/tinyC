#include "defs.h"
#include "data.h"
#include "decl.h"

//AST tree Optimisation Code

//Fold AST tree with binary operator and two A_INTLIT children,Return either original tree or new leaf node.
static ast *fold2(ast *n) 
{
  	int val, leftval, rightval;

  	// Get the values from each child
  	leftval = n->left->a_intvalue;
  	rightval = n->right->a_intvalue;

  	//Perform some of the binary operations. For any AST op we can't do, return the original tree.
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
      			// Don't try to divide by zero.
      			if (rightval == 0)
				return (n);
      			val = leftval / rightval;
      			break;
    		default:
      			return (n);
  		}

  	// Return a leaf node with the new value
  	return (mkastleaf(A_INTLIT, n->type, NULL, NULL, val));
}

//Fold AST tree with unary operator and one INTLIT children. Return either original tree or new leaf node.
static ast *fold1(ast *n) 
{
  	int val;

  	// Get the child value. Do the operation if recognised. Return the new leaf node.
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

  	// Return a leaf node with the new value
  	return (mkastleaf(A_INTLIT, n->type, NULL, NULL, val));
}

// Attempt to do constant folding on the AST tree with the root node n
static ast *fold(ast *n) 
{
  	if(n == NULL)
    		return (NULL);

  	// Fold on the left child, then do the same on the right child
  	n->left = fold(n->left);
  	n->right = fold(n->right);

  	// If both children are A_INTLITs, do a fold2()
  	if(n->left && n->left->op == A_INTLIT) 
  	{
    		if (n->right && n->right->op == A_INTLIT)
      			n = fold2(n);
    		else
     			// If only the left is A_INTLIT, do a fold1()
      			n = fold1(n);
  	}
  	// Return the possibly modified tree
  	return n;
}

// Optimise an AST tree by constant folding in all sub-trees
ast *optimise(ast *n)
{
  	n = fold(n);
  	return n;
}
