#include "defs.h"
#include "data.h"
#include "decl.h"

//this func will generate assembly code recursively..
int genAST(ast *n, int reg)
{
	int leftreg, rightreg;
	
	//get the left and right sub-tree values..

	if(n->left)
		leftreg = genAST(n->left, -1);	
	if(n->right)
		rightreg = genAST(n->right, leftreg);
	
	switch(n->op)
	{
		case A_ADD :
			return (cgadd(leftreg,rightreg));
		case A_SUBTRACT : 
			return (cgsub(leftreg,rightreg));
		case A_MULTIPLY : 
			return (cgmul(leftreg,rightreg));
		case A_DIVIDE : 	
			return (cgdiv(leftreg,rightreg));
		case A_INTLIT : 
			return (cgloadint(n->v.intvalue));
		case A_IDENT :
			return (cgloadglob(Gsym[n->v.id].name));
		case A_LVIDENT:
    			return (cgstorglob(reg, Gsym[n->v.id].name));
  		case A_ASSIGN:
    			//the work has already been done, return the result
    			return (rightreg);
		default : 
			fatald("Unknown AST operator", n->op);
	}	
}

void genpreamble() 
{
  	cgpreamble();
}
void genpostamble() 
{
  	cgpostamble();
}
void genfreeregs() 
{
  	freeall_registers();
}
void genprintint(int reg)
{
  	cgprintint(reg);
}

void genglobsym(char *s)
{
	cgglobsym(s);
}
