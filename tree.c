#include "defs.h"
#include "data.h"
#include "decl.h"

ast *mkastnode(int op, ast *left, ast *right, int intvalue)
{
	ast *n;

	n = (ast *)malloc(sizeof(ast));
	if(n == NULL)
	{
		fprintf(stderr,"Unable to malloc in mkastnode()\n");
		exit(1);
	}
	//copy the data and return it
	n -> op = op;
	n -> left = left;
	n -> right = right;
	n -> intvalue = intvalue;

	return (n);
}

ast *mkastleaf(int op, int intvalue) {
  return (mkastnode(op, NULL, NULL, intvalue));
}

// Make a unary AST node: only one child
ast *mkastunary(int op, ast *left, int intvalue) {
  return (mkastnode(op, left, NULL, intvalue));
}
