#include "defs.h"
#include "data.h"
#include "decl.h"

//generic code generator

//generate new lable number and return it..
int genlabel(void) 
{
  	static int id = 1;
  	return id++;
}

//generate the code for an IF statement and an optional ELSE clause
static int genIF(ast *n) 
{
  	int Lfalse, Lend;

  	//generate two labels: one for the false compound statement, 
	//and one for end of the overall IF statement, when there is no ELSE clause, Lfalse _is_ ending label!
  	Lfalse = genlabel();
  	
	if(n->right)
    		Lend = genlabel();

  	//generate the condition code followed by a jump to the false label.
  	genAST(n->left, Lfalse, n->op);
  	genfreeregs();

  	//generate the true compound statement
  	genAST(n->mid, NOLABEL, n->op);
  	genfreeregs();

  	//if there is an optional ELSE clause,
  	//generate the jump to skip to the end
  	if(n->right)
    		cgjump(Lend);

  	//now the false label
  	cglabel(Lfalse);

  	//optional ELSE clause: generate the false compound statement and the end label
  	if(n->right) 
	{
    		genAST(n->right, NOLABEL, n->op);
    		genfreeregs();
    		cglabel(Lend);
  	}

  	return NOREG;
}

//generate the code for a WHILE statement
static int genWHILE(ast *n) 
{
  	int Lstart, Lend;

  	//generate the start and end labels and output the start label
  	Lstart = genlabel();
  	Lend = genlabel();
  	cglabel(Lstart);

  	//generate the condition code followed by a jump to the end label.
  	genAST(n->left, Lend, n->op);
  	genfreeregs();

  	//generate the compound statement for the body
  	genAST(n->right, NOLABEL, n->op);
  	genfreeregs();

  	//finally output the jump back to the condition, and the end label
  	cgjump(Lstart);
  	cglabel(Lend);
  	return NOREG;
}

//given an AST, an optional label, and the AST op of the parent, generate assembly code recursively.
//return the register id with the tree's final value.
int genAST(ast *n, int label, int parentASTop) 
{
  	int leftreg, rightreg;

  	//we have some specific AST node handling at the top 
	//so that we don't evaluate the child sub-trees immediately
  	switch(n->op) 
	{
    		case A_IF:
      			return genIF(n);
    		case A_WHILE:
      			return genWHILE(n);
    		case A_GLUE:
      			//do each child statement, and free the registers after each child
      			genAST(n->left, NOLABEL, n->op);
      			genfreeregs();
      			genAST(n->right, NOLABEL, n->op);
      			genfreeregs();
      			return NOREG;
    		case A_FUNCTION:
      			//generate the function's preamble before the code in the child sub-tree
      			cgfuncpreamble(n->v.id);
      			genAST(n->left, NOLABEL, n->op);
      			cgfuncpostamble(n->v.id);
      			return NOREG;
  	}

  	//general AST node handling below

  	//get the left and right sub-tree values
  	if(n->left)
    		leftreg = genAST(n->left, NOLABEL, n->op);
  	if(n->right)
    		rightreg = genAST(n->right, NOLABEL, n->op);

  	switch(n->op) 
	{
    		case A_ADD:
      			return cgadd(leftreg, rightreg);
    		case A_SUBTRACT:
      			return cgsub(leftreg, rightreg);
    		case A_MULTIPLY:
      			return cgmul(leftreg, rightreg);
    		case A_DIVIDE:
      			return cgdiv(leftreg, rightreg);
    		
		case A_EQ:
    		case A_NE:
    		case A_LT:
    		case A_GT:
    		case A_LE:
    		case A_GE:
      			//if the parent AST node is an A_IF or A_WHILE, generate
      			//a compare followed by a jump. Otherwise, compare registers
      			//and set one to 1 or 0 based on the comparison.
      			if(parentASTop == A_IF || parentASTop == A_WHILE)
				return cgcompare_and_jump(n->op, leftreg, rightreg, label);
      			else
				return cgcompare_and_set(n->op, leftreg, rightreg);
    		case A_INTLIT:
      			return cgloadint(n->v.intvalue, n->type);
    		case A_IDENT:
      			//load our value if we are an rvalue or we are being dereferenced
      			if(n->rvalue || parentASTop== A_DEREF)
        			return cgloadglob(n->v.id);
      			else
        			return NOREG;
    		case A_ASSIGN:
      			//are we assigning to an identifier or through a pointer?
      			switch(n->right->op) 
			{
        			case A_IDENT: 
					return cgstorglob(leftreg, n->right->v.id);
				case A_DEREF:
				       	return cgstorderef(leftreg, rightreg, n->right->type);
        			default:
				       	fatald("Can't A_ASSIGN in genAST(), op", n->op);
      			}
    		case A_WIDEN:
      			//widen the child's type to the parent's type
      			return cgwiden(leftreg, n->left->type, n->type);
    		case A_RETURN:
      			cgreturn(leftreg, Functionid);
      			return NOREG;
    		case A_FUNCCALL:
      			return cgcall(leftreg, n->v.id);
    		case A_ADDR:
      			return cgaddress(n->v.id);
    		case A_DEREF:
      			//if we are an rvalue, dereference to get the value we point at,
      			//otherwise leave it for A_ASSIGN to store through the pointer
      			if(n->rvalue)
        			return cgderef(leftreg, n->left->type);
      			else
        			return leftreg;
    		case A_SCALE:
      			//small optimisation: use shift if the
      			//scale value is a known power of two
      			switch(n->v.size) 
			{
				case 2: return(cgshlconst(leftreg, 1));
				case 4: return(cgshlconst(leftreg, 2));
				case 8: return(cgshlconst(leftreg, 3));
				default:
	  				//load a register with the size and
	  				//multiply the leftreg by this size
          				rightreg= cgloadint(n->v.size, P_INT);
          				return cgmul(leftreg, rightreg);
      			}
    		default:
      			fatald("Unknown AST operator", n->op);
  	}
  	return NOREG;
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
void genglobsym(int id) 
{
  	cgglobsym(id);
}
int genprimsize(int type) 
{
  	return cgprimsize(type);
}
