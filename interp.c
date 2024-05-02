#include "defs.h"
#include "data.h"
#include "decl.h"	

//list of AST operators
static char *ASTop[] = { "+", "-", "*", "/" };

//given an AST, interpret the operators in it and return a final value.
int interpretAST(ast *n) 
{
    int leftval, rightval;

    //get the left and right sub-tree values
    if (n->left)
        leftval = interpretAST(n->left);
    if (n->right)
        rightval = interpretAST(n->right);
  
    //debug: print what we are about to do
    //if(n->op == A_INTLIT)
      //printf("int %d\n", n->intvalue);
    //else
      //printf("%d %s %d\n", leftval, ASTop[n->op], rightval);

    switch (n->op) 
    {
        case A_ADD:
          return (leftval + rightval);
        case A_SUBTRACT:
          return (leftval - rightval);
        case A_MULTIPLY:
          return (leftval * rightval);
        case A_DIVIDE:
          return (leftval / rightval);
        case A_INTLIT:
          return (n->intvalue);
        default:
          fprintf(stderr, "Unknown AST operator %d\n", n->op);
          exit(1);
  } 
}
