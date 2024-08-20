#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//generic code generator from parsed tree

static int labelid = 1;
int genlabel(void) 
{
  	return labelid++;
}

static void update_line(ast *n) 
{
  	if(n->linenum != 0 && Line != n->linenum) 
  	{
    		Line = n->linenum;
    		cglinenum(Line);
  	}
}

static int genIF(ast *n, int looptoplabel, int loopendlabel) 
{
  	int Lfalse, Lend = 0;
	int r, r2;
	
  	Lfalse = genlabel();
  	if(n->right)
    		Lend = genlabel();

  	r = genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);

	r2 = cgloadint(1, P_INT);
  	cgcompare_and_jump(A_EQ, r, r2, Lfalse, P_INT);
  	
	genAST(n->mid, NOLABEL, looptoplabel, loopendlabel, n->op);


  	if(n->right)
  	{
		cglabel(genlabel());
    		cgjump(Lend);
	}
  	cglabel(Lfalse);

  	if(n->right) 
  	{
    		genAST(n->right, NOLABEL, NOLABEL, loopendlabel, n->op);
    		cglabel(Lend);
  	}

  	return NOREG;
}

static int genWHILE(ast *n)
{
  	int Lstart, Lend;
  	int r, r2;
  	
	Lstart = genlabel();
  	Lend = genlabel();
  	cglabel(Lstart);

	r = genAST(n->left, Lend, Lstart, Lend, n->op);
  	r2 = cgloadint(1, P_INT);
  	cgcompare_and_jump(A_EQ, r, r2, Lend, P_INT);

  	genAST(n->right, NOLABEL, Lstart, Lend, n->op);

  	cgjump(Lstart);
  	cglabel(Lend);
  	
  	return NOREG;
}

static int genSWITCH(ast *n) 
{
  	int *caselabel;
  	int Lend;
  	int Lcode = 0;
  	int i, reg, r2, type;
  	ast *c;

  	caselabel = (int *) malloc((n->a_intvalue + 1) * sizeof(int));
  	if(caselabel == NULL)
    		fatal("malloc failed in genSWITCH");

  	Lend = genlabel();

  	for(i = 0, c = n->right; c != NULL; i++, c = c->right)
    		caselabel[i] = genlabel();
  
  	caselabel[i] = Lend;

  	reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
  	type = n->left->type;

  	for(i = 0, c = n->right; c != NULL; i++, c = c->right) 
  	{

    		if(Lcode == 0)
      			Lcode = genlabel();

    		cglabel(caselabel[i]);

    		if(c->op != A_DEFAULT) 
    		{
      			r2 = cgloadint(c->a_intvalue, type);
      			cgcompare_and_jump(A_EQ, reg, r2, caselabel[i + 1], type);

      			cgjump(Lcode);
    		}
    		if(c->left) 
    		{
      			cglabel(Lcode);
      			genAST(c->left, NOLABEL, NOLABEL, Lend, 0);
      			Lcode = 0;
    		}
  	}

  	cglabel(Lend);
  	
  	return NOREG;
}

static int gen_logandor(ast *n) 
{
  	int Lfalse = genlabel();
  	int Lend = genlabel();
  	int reg;
  	int type;

  	reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
  	type =n -> left -> type;
  	cgboolean(reg, n->op, Lfalse, type);
  	
	reg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, 0);
	type = n->right->type;
  	cgboolean(reg, n->op, Lfalse, type);

  	if(n->op== A_LOGAND) 
  	{
    		cgloadboolean(reg, 1, type);
    		cgjump(Lend);
    		cglabel(Lfalse);
    		cgloadboolean(reg, 0, type);
  	}
  	else 
  	{
    		cgloadboolean(reg, 0, type);
    		cgjump(Lend);
    		cglabel(Lfalse);
    		cgloadboolean(reg, 1, type);
  	}	
  	cglabel(Lend);
  	
  	return reg;
}

static int gen_funccall(ast *n) 
{
  	ast *gluetree;
  	int i = 0, numargs = 0;
  	int *arglist = NULL;
  	int *typelist = NULL;
  	
  	for(gluetree = n->left; gluetree != NULL; gluetree = gluetree->left) 
  	{
    		numargs++;
  	}

  	for(i = 0, gluetree = n->left; gluetree != NULL; gluetree = gluetree->left)
    		i++;

	if(i != 0) 
  	{
    		arglist = (int *) malloc(i * sizeof(int));
    		if(arglist == NULL)
      			fatal("malloc failed in gen_funccall");
    		
    		typelist = (int *) malloc(i * sizeof(int));
    		if(typelist == NULL)
      			fatal("malloc failed in gen_funccall");
  	}
  	for(i = 0, gluetree = n->left; gluetree != NULL; gluetree = gluetree->left) 
  	{
    		arglist[i] =
      		genAST(gluetree->right, NOLABEL, NOLABEL, NOLABEL, gluetree->op);
    		typelist[i++] = gluetree->right->type;
  	}

  	return cgcall(n->sym, numargs, arglist, typelist);
}

static int gen_ternary(ast *n) 
{
  	int Lfalse, Lend;
  	int reg, expreg;
  	int r, r2;

  	Lfalse = genlabel();
  	Lend = genlabel();

  	r = genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);
  	r2 = cgloadint(1, P_INT);
  	cgcompare_and_jump(A_EQ, r, r2, Lfalse, P_INT);

  	reg = cgalloctemp();

  	expreg = genAST(n->mid, NOLABEL, NOLABEL, NOLABEL, n->op);
  	cgmove(expreg, reg, n->mid->type);
  	cgjump(Lend);
  	cglabel(Lfalse);

  	expreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);
  	cgmove(expreg, reg, n->right->type);
  	cglabel(Lend);
  	
  	return reg;
}

int genAST(ast *n, int iflabel, int looptoplabel, int loopendlabel, int parentASTop) 
{
  	int leftreg = NOREG, rightreg = NOREG;
  	int lefttype = P_VOID, type = P_VOID;
  	symt *leftsym = NULL;

  	if(n == NULL)
    		return NOREG;

  	update_line(n);

  	switch(n->op) 
  	{
    		case A_IF:
      			return genIF(n, looptoplabel, loopendlabel);
    		case A_WHILE:
      			return genWHILE(n);
    		case A_SWITCH:
      			return genSWITCH(n);
    		case A_FUNCCALL:
      			return gen_funccall(n);
    		case A_TERNARY:
      			return gen_ternary(n);
    		case A_LOGOR:
      			return gen_logandor(n);
    		case A_LOGAND:
      			return gen_logandor(n);
    		case A_GLUE:
      			if(n->left != NULL)
				genAST(n->left, iflabel, looptoplabel, loopendlabel, n->op);
      			if(n->right != NULL)
				genAST(n->right, iflabel, looptoplabel, loopendlabel, n->op);
      			return NOREG;
    		case A_FUNCTION:
      			cgfuncpreamble(n->sym);
      			genAST(n->left, NOLABEL, NOLABEL, NOLABEL, n->op);
      			cgfuncpostamble(n->sym);
      			return NOREG;
  	}

  	if(n->left) 
  	{
    		lefttype = type = n->left->type;
    		leftsym = n->left->sym;
    		leftreg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, n->op);
  	}
  	
  	if(n->right) 
  	{
    		type = n->right->type;
    		rightreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);
  	}

  	switch(n->op) 
  	{
    		case A_ADD:
      			return cgadd(leftreg, rightreg, type);
    		case A_SUBTRACT:
      			return cgsub(leftreg, rightreg, type);
    		case A_MULTIPLY:
      			return cgmul(leftreg, rightreg, type);
    		case A_DIVIDE:
      			return cgdivmod(leftreg, rightreg, A_DIVIDE, type);
    		case A_MOD:
      			return cgdivmod(leftreg, rightreg, A_MOD, type);
    		case A_AND:
      			return cgand(leftreg, rightreg, type);
    		case A_OR:
      			return cgor(leftreg, rightreg, type);
    		case A_XOR:
      			return cgxor(leftreg, rightreg, type);
    		case A_LSHIFT:
      			return cgshl(leftreg, rightreg, type);
    		case A_RSHIFT:
      			return cgshr(leftreg, rightreg, type);
    		case A_EQ:
    		case A_NE:
    		case A_LT:
    		case A_GT:
    		case A_LE:
    		case A_GE:
      			return cgcompare_and_set(n->op, leftreg, rightreg, lefttype);
    		case A_INTLIT:
      			return cgloadint(n->a_intvalue, n->type);
    		case A_STRLIT:
      			return cgloadglobstr(n->a_intvalue);
    		case A_IDENT:
      			if(n->rvalue || parentASTop == A_DEREF) 
      			{
				return cgloadvar(n->sym, n->op);
      			} 
      			else
				return NOREG;
    		case A_ASPLUS:
    		case A_ASMINUS:
    		case A_ASSTAR:
    		case A_ASSLASH:
    		case A_ASMOD:
    		case A_ASSIGN:

      			switch(n->op) 
      			{
				case A_ASPLUS:
	  				leftreg = cgadd(leftreg, rightreg, type);
	  				n->right = n->left;
	  				break;
				case A_ASMINUS:
	  				leftreg = cgsub(leftreg, rightreg, type);
	  				n->right = n->left;
	  				break;
				case A_ASSTAR:
	 				leftreg = cgmul(leftreg, rightreg, type);
	  				n->right = n->left;
	  				break;
				case A_ASSLASH:
	  				leftreg = cgdivmod(leftreg, rightreg, A_DIVIDE, type);
	  				n->right = n->left;
	  				break;
				case A_ASMOD:
	  				leftreg = cgdivmod(leftreg, rightreg, A_MOD, type);
	  				n->right = n->left;
	  				break;
      			}

      			switch(n->right->op) 
      			{
				case A_IDENT:
	  				if(n->right->sym->class == C_GLOBAL ||
	      				   n->right->sym->class == C_EXTERN ||
	      				   n->right->sym->class == C_STATIC)
  					   return cgstorglob(leftreg, n->right->sym);
	  				else
	    				  	return cgstorlocal(leftreg, n->right->sym);
				case A_DEREF:
	  				return cgstorderef(leftreg, rightreg, n->right->type);
				default:
	  				fatald("Can't A_ASSIGN in genAST(), op", n->op);
      			}
    		case A_WIDEN:
      			return cgwiden(leftreg, lefttype, n->type);
    		case A_RETURN:
      			cgreturn(leftreg, Functionid);
      			return NOREG;
    		case A_ADDR:
      			if(n->sym != NULL)
				return cgaddress(n->sym);
      			else
				return leftreg;
    		case A_DEREF:
      			if(n->rvalue)
				return cgderef(leftreg, lefttype);
      			else
				return leftreg;
    		case A_SCALE:
      			switch(n->a_size) 
      			{
				case 2:
	  				return cgshlconst(leftreg, 1, type);
				case 4:
	  				return cgshlconst(leftreg, 2, type);
				case 8:
	  				return cgshlconst(leftreg, 3, type);
				default:
	  				rightreg = cgloadint(n->a_size, P_INT);
	  				return cgmul(leftreg, rightreg, type);
      			}
    		
    		case A_POSTINC:
    		case A_POSTDEC:
      			return cgloadvar(n->sym, n->op);
    		case A_PREINC:
    		case A_PREDEC:
      			return cgloadvar(leftsym, n->op);
    		case A_NEGATE:
      			return cgnegate(leftreg, type);
    		case A_INVERT:
      			return cginvert(leftreg, type);
    		case A_LOGNOT:
      			return cglognot(leftreg, type);
    		case A_TOBOOL:
      			return cgboolean(leftreg, parentASTop, iflabel, type);
    		case A_BREAK:
      			cgjump(loopendlabel);
      			return NOREG;
    		case A_CONTINUE:
      			cgjump(looptoplabel);
      			return NOREG;
    		case A_CAST:
      			return cgcast(leftreg, lefttype, n->type);
    		default:
      			fatald("Unknown AST operator", n->op);
  	}
  	return NOREG;	
}

void genpreamble(char *filename) 
{
  	cgpreamble(filename);
}

void genpostamble() 
{
  	cgpostamble();
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
