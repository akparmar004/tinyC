#include "defs.h"
#include "data.h"
#include "decl.h"

//AST tree functions

//build a simple AST node and return it..
ast *mkastnode(int op, ast *left, ast *mid, ast *right, int intvalue) 
{
  	ast *n;

  	//malloc a new ASTnode
  	n = (ast *) malloc(sizeof(ast));
  	if (n == NULL)
		fatal("Unable to malloc in mkastnode()");

 	//copy all the values and return it
  	n -> op = op;
  	n -> left = left;
  	n -> mid = mid;
  	n -> right = right;
  	n -> v.intvalue = intvalue;
  	return n;
}


//make a unary AST node which is having only one child
ast *mkastunary(int op, ast *left, int intvalue) 
{
 	return mkastnode(op, left, NULL, NULL, intvalue);
}

//make an AST leaf node
ast *mkastleaf(int op, int intvalue) 
{
  	return mkastnode(op, NULL, NULL, NULL, intvalue);
}
