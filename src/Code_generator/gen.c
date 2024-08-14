#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//generic code generator from parsed tree

static int labelid = 1;
int genlabel(void) 
{
  	return labelid++;
}

static int genIF(ast *n, int looptoplabel, int loopendlabel) 
{
  	int Lfalse, Lend;

  	Lfalse = genlabel();
  	if(n->right)
    		Lend = genlabel();

  	genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);
  	genfreeregs(NOREG);

  	genAST(n->mid, NOLABEL, looptoplabel, loopendlabel, n->op);
  	genfreeregs(NOREG);

  	if(n->right)
    		cgjump(Lend);

  	cglabel(Lfalse);

  	if(n->right) 
  	{
    		genAST(n->right, NOLABEL, NOLABEL, loopendlabel, n->op);
    		genfreeregs(NOREG);
    		cglabel(Lend);
  	}

  	return NOREG;
}

static int genWHILE(ast *n)
{
  	int Lstart, Lend;
  	
	Lstart = genlabel();
  	Lend = genlabel();
  	cglabel(Lstart);

  	genAST(n->left, Lend, Lstart, Lend, n->op);
  	genfreeregs(NOREG);

  	genAST(n->right, NOLABEL, Lstart, Lend, n->op);
  	genfreeregs(NOREG);

  	cgjump(Lstart);
  	cglabel(Lend);
  	
  	return NOREG;
}

static int genSWITCH(ast *n) 
{
  	int *caseval, *caselabel;
  	int Ljumptop, Lend;
  	int i, reg, defaultlabel = 0, casecount = 0;
  	ast *c;

  	caseval = (int *) malloc((n->a_intvalue + 1) * sizeof(int));
  	caselabel = (int *) malloc((n->a_intvalue + 1) * sizeof(int));

  	Ljumptop = genlabel();
  	Lend = genlabel();
  	defaultlabel = Lend;

  	reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
  	cgjump(Ljumptop);
  	genfreeregs(reg);

  	for(i = 0, c = n->right; c != NULL; i++, c = c->right) 
  	{

    		caselabel[i] = genlabel();
    		caseval[i] = c->a_intvalue;
    		cglabel(caselabel[i]);
    		if(c->op == A_DEFAULT)
      			defaultlabel = caselabel[i];
    		else	
      			casecount++;

    		if(c->left)
      			genAST(c->left, NOLABEL, NOLABEL, Lend, 0);
    		
    		genfreeregs(NOREG);
  	}

  	cgjump(Lend);

  	cgswitch(reg, casecount, Ljumptop, caselabel, caseval, defaultlabel);
  	cglabel(Lend);
  	
  	return NOREG;
}

static int gen_logandor(ast *n) 
{
  	int Lfalse = genlabel();
  	int Lend = genlabel();
  	int reg;

  	reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
  	cgboolean(reg, n->op, Lfalse);
  	genfreeregs(NOREG);
  	
	reg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, 0);
  	cgboolean(reg, n->op, Lfalse);
  	genfreeregs(reg);

  	if(n->op== A_LOGAND) 
  	{
    		cgloadboolean(reg, 1);
    		cgjump(Lend);
    		cglabel(Lfalse);
    		cgloadboolean(reg, 0);
  	}
  	else 
  	{
    		cgloadboolean(reg, 0);
    		cgjump(Lend);
    		cglabel(Lfalse);
    		cgloadboolean(reg, 1);
  	}	
  	cglabel(Lend);
  	
  	return reg;
}

static int gen_funccall(ast *n) 
{
  	ast *gluetree = n->left;
  	int reg;
  	int numargs = 0;

  	spill_all_regs();

  	while (gluetree) 
  	{
    		reg = genAST(gluetree->right, NOLABEL, NOLABEL, NOLABEL, gluetree->op);
    		
    		cgcopyarg(reg, gluetree->a_size);
    			
    		if(numargs == 0)
      			numargs = gluetree->a_size;
    		
    		gluetree = gluetree->left;
  	}

  	return (cgcall(n->sym, numargs));
}

static int gen_ternary(ast *n) 
{
  	int Lfalse, Lend;
  	int reg, expreg;

  	Lfalse = genlabel();
  	Lend = genlabel();

  	genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);
  	genfreeregs(NOREG);

  	reg = alloc_register();

  	expreg = genAST(n->mid, NOLABEL, NOLABEL, NOLABEL, n->op);
  	cgmove(expreg, reg);
  	genfreeregs(reg);
  	cgjump(Lend);
  	cglabel(Lfalse);

  	expreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);
  	cgmove(expreg, reg);
  	genfreeregs(reg);
  	cglabel(Lend);
  	
  	return reg;
}

int genAST(ast *n, int iflabel, int looptoplabel, int loopendlabel, int parentASTop) 
{
  	int leftreg= NOREG, rightreg= NOREG;

  	if(n==NULL) return(NOREG);

  	switch(n->op) 
  	{
    		case A_IF:
      			return genIF(n, looptoplabel, loopendlabel);
    		case A_WHILE:
      			return (genWHILE(n));
    		case A_SWITCH:
      			return (genSWITCH(n));
    		case A_FUNCCALL:
      			return (gen_funccall(n));
    		case A_TERNARY:
      			return (gen_ternary(n));
    		case A_LOGOR:
      			return (gen_logandor(n));
    		case A_LOGAND:
      			return (gen_logandor(n));
    		case A_GLUE:
      			if(n->left != NULL)
				genAST(n->left, iflabel, looptoplabel, loopendlabel, n->op);
      			
      			genfreeregs(NOREG);
      			if(n->right != NULL)
				genAST(n->right, iflabel, looptoplabel, loopendlabel, n->op);
      			genfreeregs(NOREG);
      			return (NOREG);
    		case A_FUNCTION:
      			cgfuncpreamble(n->sym);
      			genAST(n->left, NOLABEL, NOLABEL, NOLABEL, n->op);
      			cgfuncpostamble(n->sym);
      			return NOREG;
  	}
  	if (n->left)
    		leftreg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, n->op);
  	if (n->right)
    		rightreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);

  	switch (n->op) 
  	{
    		case A_ADD:
      			return (cgadd(leftreg, rightreg));
    		case A_SUBTRACT:
      			return (cgsub(leftreg, rightreg));
    		case A_MULTIPLY:
      			return (cgmul(leftreg, rightreg));
    		case A_DIVIDE:
      			return (cgdivmod(leftreg, rightreg, A_DIVIDE));
    		case A_MOD:
      			return (cgdivmod(leftreg, rightreg, A_MOD));
    		case A_AND:
      			return (cgand(leftreg, rightreg));
    		case A_OR:
      			return (cgor(leftreg, rightreg));
    		case A_XOR:
      			return (cgxor(leftreg, rightreg));
    		case A_LSHIFT:
      			return (cgshl(leftreg, rightreg));
    		case A_RSHIFT:
      			return (cgshr(leftreg, rightreg));
    		case A_EQ:
    		case A_NE:
    		case A_LT:
    		case A_GT:
    		case A_LE:
    		case A_GE:
      			if (parentASTop == A_IF || parentASTop == A_WHILE || parentASTop == A_TERNARY)
				return (cgcompare_and_jump(n->op, leftreg, rightreg, iflabel));
      			else
				return (cgcompare_and_set(n->op, leftreg, rightreg));
    		case A_INTLIT:
      			return (cgloadint(n->a_intvalue, n->type));
    		case A_STRLIT:
      			return (cgloadglobstr(n->a_intvalue));
    		case A_IDENT:
     			if (n->rvalue || parentASTop == A_DEREF) 
     			{
				return (cgloadvar(n->sym, n->op));
      			}
      			else
				return (NOREG);
    		case A_ASPLUS:
    		case A_ASMINUS:
   		case A_ASSTAR:
    		case A_ASSLASH:
    		case A_ASMOD:
    		case A_ASSIGN:

      			switch (n->op) 
      			{
				case A_ASPLUS:
	  				leftreg = cgadd(leftreg, rightreg);
	  				n->right = n->left;
	  				break;
				case A_ASMINUS:
	  				leftreg = cgsub(leftreg, rightreg);
	  				n->right = n->left;
	  				break;
				case A_ASSTAR:
	  				leftreg = cgmul(leftreg, rightreg);
	  				n->right = n->left;
	  				break;
				case A_ASSLASH:
	  				leftreg = cgdivmod(leftreg, rightreg, A_DIVIDE);
	  				n->right = n->left;
	  				break;
				case A_ASMOD:
	  				leftreg = cgdivmod(leftreg, rightreg, A_MOD);
	  				n->right = n->left;
	  				break;
      			}

      			switch (n->right->op) 
      			{
				case A_IDENT:
	  				if (n->right->sym->class == C_GLOBAL || n->right->sym->class == C_STATIC)
	    					return (cgstorglob(leftreg, n->right->sym));
	  				else
	    					return (cgstorlocal(leftreg, n->right->sym));
				case A_DEREF:
	  				return (cgstorderef(leftreg, rightreg, n->right->type));
				default:
	  				fatald("Can't A_ASSIGN in genAST(), op", n->op);
      			}
    		case A_WIDEN:
      			return (cgwiden(leftreg, n->left->type, n->type));
    		case A_RETURN:
      			cgreturn(leftreg, Functionid);
      			return (NOREG);
    		case A_ADDR:
      			return (cgaddress(n->sym));
    		case A_DEREF:
      			if (n->rvalue)
				return (cgderef(leftreg, n->left->type));
      			else
				return (leftreg);
    		case A_SCALE:
      			switch (n->a_size) 
      			{
				case 2:
	  				return (cgshlconst(leftreg, 1));
				case 4:
	  				return (cgshlconst(leftreg, 2));
				case 8:
	  				return (cgshlconst(leftreg, 3));
				default:
	  				rightreg = cgloadint(n->a_size, P_INT);
	  				return (cgmul(leftreg, rightreg));
      			}
    		case A_POSTINC:
    		case A_POSTDEC:
      			return (cgloadvar(n->sym, n->op));
    		case A_PREINC:
    		case A_PREDEC:
      			return (cgloadvar(n->left->sym, n->op));
    		case A_NEGATE:
      			return (cgnegate(leftreg));
    		case A_INVERT:
      			return (cginvert(leftreg));
    		case A_LOGNOT:
      			return (cglognot(leftreg));
    		case A_TOBOOL:
      			return (cgboolean(leftreg, parentASTop, iflabel));
    		case A_BREAK:
      			cgjump(loopendlabel);
      			return (NOREG);
    		case A_CONTINUE:
      			cgjump(looptoplabel);
      			return (NOREG);
    		case A_CAST:
      			return (leftreg);		// Not much to do
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
void genfreeregs(int keepreg) 
{
  	freeall_registers(keepreg);
}
void genglobsym(symt *node) 
{
  	cgglobsym(node);
}

int genglobstr(char *strvalue, int append) 
{
  	int l = genlabel();
  	cgglobstr(l, strvalue, append);
  	
  	return l;
}
void genglobstrend(void) 
{
  	cgglobstrend();
}
int genprimsize(int type) 
{
  	return cgprimsize(type);
}
int genalign(int type, int offset, int direction) 
{
  	return cgalign(type, offset, direction);
}
