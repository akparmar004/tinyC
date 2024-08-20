#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//code generator for x86-64 using the QBE IL...

void cgtextseg() 
{
}

void cgdataseg() 
{
}

char cgqbetype(int type) 
{
  	if(ptrtype(type))
    		return ('l');
  	switch(type)
	{
    		case P_VOID:
      			return (' ');
    		case P_CHAR:
      			return ('w');
    		case P_INT:
      			return ('w');
    		case P_LONG:
      			return ('l');
    		default:
      			fatald("Bad type in cgqbetype:", type);
  	}
  	return (0);	
}

int cgprimsize(int type) 
{
  	if(ptrtype(type))
    		return (8);
  	
	switch(type) 
	{
    		case P_CHAR:
      			return (1);
    		case P_INT:
      			return (4);
    		case P_LONG:
      			return (8);
    		default:
      			fatald("Bad type in cgprimsize:", type);
  	}
  	return (0);	
}

int cgalign(int type, int offset, int direction) 
{
  	int alignment;

  	switch(type) 
	{
    		case P_CHAR:
      			break;
    		default:
      			alignment = 4;
      			offset = (offset + direction * (alignment - 1)) & ~(alignment - 1);
  	}
  	return offset;
}

static int nexttemp = 0;
int cgalloctemp(void) 
{
  	return ++nexttemp;
}

void cgpreamble(char *filename) 
{
}

void cgpostamble() 
{
}

static int used_switch;

void cgfuncpreamble(symt *sym) 
{
	char *name = sym->name;
  	symt *parm, *locvar;
  	int size, bigsize;
  	int label;

  	if(sym->class == C_GLOBAL)
    		fprintf(Outfile, "export ");
  	
	fprintf(Outfile, "function %c $%s(", cgqbetype(sym->type), name);

  	for(parm = sym->member; parm != NULL; parm = parm->next) 
	{
    		if(parm->st_hasaddr == 1)
      			fprintf(Outfile, "%c %%.p%s, ", cgqbetype(parm->type), parm->name);
    		else
      			fprintf(Outfile, "%c %%%s, ", cgqbetype(parm->type), parm->name);
  	}
  	fprintf(Outfile, ") {\n");

  	label = genlabel();
  	cglabel(label);

  	for(parm = sym->member; parm != NULL; parm = parm->next) 
	{
    		if(parm->st_hasaddr == 1) 
		{
      			size = cgprimsize(parm->type);
      			bigsize = (size == 1) ? 4 : size;
      			fprintf(Outfile, "  %%%s =l alloc%d 1\n", parm->name, bigsize);

      			switch(size) 
			{
				case 1:
	  				fprintf(Outfile, "  storeb %%.p%s, %%%s\n", parm->name, parm->name);
	  				break;
				case 4:
	  				fprintf(Outfile, "  storew %%.p%s, %%%s\n", parm->name, parm->name);
	  				break;
				case 8:
	  				fprintf(Outfile, "  storel %%.p%s, %%%s\n", parm->name, parm->name);
      			}
    		}
  	}

  	for(locvar = Loclhead; locvar != NULL; locvar = locvar->next) 
	{
    		if(locvar->st_hasaddr == 1) 
		{
      			size = locvar->size * locvar->nelems;
      			size = (size + 7) >> 3;
      			fprintf(Outfile, "  %%%s =l alloc8 %d\n", locvar->name, size);
    		} 
		else if (locvar->type == P_CHAR) 
		{
      			locvar->st_hasaddr = 1;
      			fprintf(Outfile, "  %%%s =l alloc4 1\n", locvar->name);
    		}
  	}

  	used_switch = 0;		
}

void cgfuncpostamble(symt *sym) 
{
  	cglabel(sym->st_endlabel);

  	if(sym->type != P_VOID)
    		fprintf(Outfile, "  ret %%.ret\n}\n");
  	else
    		fprintf(Outfile, "  ret\n}\n");
}

int cgloadint(int value, int type) 
{
  	int t = cgalloctemp();

  	fprintf(Outfile, "  %%.t%d =%c copy %d\n", t, cgqbetype(type), value);
  	
  	return t;
}

int cgloadvar(symt *sym, int op) 
{
  	int r, posttemp, offset = 1;
  	char qbeprefix;

  	r = cgalloctemp();

  	if(ptrtype(sym->type))
    		offset = typesize(value_at(sym->type), sym->ctype);

  	if(op == A_PREDEC || op == A_POSTDEC)
    		offset = -offset;

  	qbeprefix = ((sym->class == C_GLOBAL) || (sym->class == C_STATIC) || (sym->class == C_EXTERN)) ? '$' : '%';

  	if(op == A_PREINC || op == A_PREDEC) 
  	{
    		if(sym->st_hasaddr || qbeprefix == '$') 
    		{
      			posttemp = cgalloctemp();
      			switch(sym->size) 
      			{
			case 1:
	  			fprintf(Outfile, "  %%.t%d =w loadub %c%s\n", posttemp, qbeprefix, sym->name);
	  			fprintf(Outfile, "  %%.t%d =w add %%.t%d, %d\n", posttemp, posttemp, offset);
				fprintf(Outfile, "  storeb %%.t%d, %c%s\n", posttemp, qbeprefix, sym->name);
	  			break;
			case 4:
	  			fprintf(Outfile, "  %%.t%d =w loadsw %c%s\n", posttemp, qbeprefix, sym->name);
	  			fprintf(Outfile, "  %%.t%d =w add %%.t%d, %d\n", posttemp, posttemp, offset);
	  			fprintf(Outfile, "  storew %%.t%d, %c%s\n", posttemp, qbeprefix, sym->name);
	  			break;
			case 8: 
				fprintf(Outfile, "  %%.t%d =l loadl %c%s\n", posttemp, qbeprefix, sym->name);
				fprintf(Outfile, "  %%.t%d =l add %%.t%d, %d\n", posttemp, posttemp, offset);
	  			fprintf(Outfile, "  storel %%.t%d, %c%s\n", posttemp, qbeprefix, sym->name);
      			}
    		} 
    		else
      			fprintf(Outfile, "  %c%s =%c add %c%s, %d\n", qbeprefix, sym->name, 
      					cgqbetype(sym->type), qbeprefix, sym->name, offset);
  	}
  	
  	if(sym->st_hasaddr || qbeprefix == '$') 
  	{
    		switch (sym->size) 
    		{
      			case 1:
				fprintf(Outfile, "  %%.t%d =w loadub %c%s\n", r, qbeprefix, sym->name);
				break;
      			case 4:
				fprintf(Outfile, "  %%.t%d =w loadsw %c%s\n", r, qbeprefix, sym->name);
				break;
      			case 8:
				fprintf(Outfile, "  %%.t%d =l loadl %c%s\n", r, qbeprefix, sym->name);
    		}
  	} 
  	else
    		fprintf(Outfile, "  %%.t%d =%c copy %c%s\n", r, cgqbetype(sym->type), qbeprefix, sym->name);

  	if(op == A_POSTINC || op == A_POSTDEC) 
  	{
    		if(sym->st_hasaddr || qbeprefix == '$') 
    		{
      			posttemp = cgalloctemp();
      			switch(sym->size) 
      			{
			case 1:
	  			fprintf(Outfile, "  %%.t%d =w loadub %c%s\n", posttemp, qbeprefix, sym->name);
	  			fprintf(Outfile, "  %%.t%d =w add %%.t%d, %d\n", posttemp, posttemp, offset);
	  			fprintf(Outfile, "  storeb %%.t%d, %c%s\n", posttemp, qbeprefix, sym->name);
	  			break;
			case 4:
	  			fprintf(Outfile, "  %%.t%d =w loadsw %c%s\n", posttemp, qbeprefix, sym->name);
	  			fprintf(Outfile, "  %%.t%d =w add %%.t%d, %d\n", posttemp, posttemp, offset);
	  			fprintf(Outfile, "  storew %%.t%d, %c%s\n", posttemp, qbeprefix, sym->name);
	  			break;
			case 8:
				fprintf(Outfile, "  %%.t%d =l loadl %c%s\n", posttemp, qbeprefix, sym->name);
	  			fprintf(Outfile, "  %%.t%d =l add %%.t%d, %d\n", posttemp, posttemp, offset);
	  			fprintf(Outfile, "  storel %%.t%d, %c%s\n", posttemp, qbeprefix, sym->name);
      			}
    		} 
    		else
      			fprintf(Outfile, "  %c%s =%c add %c%s, %d\n", qbeprefix, sym->name, 
      				cgqbetype(sym->type), qbeprefix, sym->name, offset);
  	}
  	return r;
}

int cgloadglobstr(int label) 
{
  	int r = cgalloctemp();
  	fprintf(Outfile, "  %%.t%d =l copy $L%d\n", r, label);
  	
  	return r;
}

int cgadd(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c add %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	
  	return r1;
}

int cgsub(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c sub %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	
  	return r1;
}

int cgmul(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c mul %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	return r1;
}

int cgdivmod(int r1, int r2, int op, int type) 
{
  	if (op == A_DIVIDE)
    		fprintf(Outfile, "  %%.t%d =%c div %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	else
    		fprintf(Outfile, "  %%.t%d =%c rem %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	return r1;
}

int cgand(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c and %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	
  	return r1;
}

int cgor(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c or %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  
  	return r1;
}

int cgxor(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c xor %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	
  	return r1;
}

int cgshl(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c shl %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	
  	return r1;
}

int cgshr(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c shr %%.t%d, %%.t%d\n", r1, cgqbetype(type), r1, r2);
  	
  	return r1;
}

int cgnegate(int r, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c sub 0, %%.t%d\n", r, cgqbetype(type), r);
  	
  	return r;
}

int cginvert(int r, int type) 
{
 	fprintf(Outfile, "  %%.t%d =%c xor %%.t%d, -1\n", r, cgqbetype(type), r);
  	
  	return r;
}

int cglognot(int r, int type) 
{
  	char q = cgqbetype(type);
  	fprintf(Outfile, "  %%.t%d =%c ceq%c %%.t%d, 0\n", r, q, q, r);
  	
  	return r;
}

void cgloadboolean(int r, int val, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c copy %d\n", r, cgqbetype(type), val);
}

int cgboolean(int r, int op, int label, int type) 
{

  	int label2 = genlabel();
	int r2 = cgalloctemp();

  	fprintf(Outfile, "  %%.t%d =l cne%c %%.t%d, 0\n", r2, cgqbetype(type), r);

  	switch(op) 
  	{
    		case A_IF:
    		case A_WHILE:
    		case A_LOGAND:
      			fprintf(Outfile, "  jnz %%.t%d, @L%d, @L%d\n", r2, label2, label);
      			break;
    		case A_LOGOR:
      			fprintf(Outfile, "  jnz %%.t%d, @L%d, @L%d\n", r2, label, label2);
      			break;
  	}

  	cglabel(label2);
  	return r2;
}

int cgcall(symt *sym, int numargs, int *arglist, int *typelist) 
{
  	int outr;
  	int i;

  	outr = cgalloctemp();

  	if(sym->type == P_VOID)
    		fprintf(Outfile, "  call $%s(", sym->name);
  	else
    		fprintf(Outfile, "  %%.t%d =%c call $%s(", outr, cgqbetype(sym->type), sym->name);

  	for(i = numargs - 1; i >= 0; i--) 
  	{
    		fprintf(Outfile, "%c %%.t%d, ", cgqbetype(typelist[i]), arglist[i]);
  	}
  	fprintf(Outfile, ")\n");

  	return outr;
}

int cgshlconst(int r, int val, int type) 
{
  	int r2 = cgalloctemp();
  	int r3 = cgalloctemp();

  	if(cgprimsize(type) < 8) 
  	{
    		fprintf(Outfile, "  %%.t%d =l extsw %%.t%d\n", r2, r);
    		fprintf(Outfile, "  %%.t%d =l shl %%.t%d, %d\n", r3, r2, val);
  	} 
  	else
    		fprintf(Outfile, "  %%.t%d =l shl %%.t%d, %d\n", r3, r, val);
  	return r3;
}

int cgstorglob(int r, symt *sym) 
{
  	char q = cgqbetype(sym->type);
  	if(sym->type == P_CHAR)
    		q = 'b';

  	fprintf(Outfile, "  store%c %%.t%d, $%s\n", q, r, sym->name);
  	
  	return r;
}

int cgstorlocal(int r, symt *sym) 
{
  	if(sym->st_hasaddr) 
  	{
    		fprintf(Outfile, "  store%c %%.t%d, %%%s\n",
	    	cgqbetype(sym->type), r, sym->name);
  	} 
  	else 
  	{
    		fprintf(Outfile, "  %%%s =%c copy %%.t%d\n", sym->name, cgqbetype(sym->type), r);
  	}
  	return r;
}

void cgglobsym(symt *node) 
{
  	int size, type;
  	int initvalue;
  	int i;

  	if(node == NULL)
    		return;
  	if(node->stype == S_FUNCTION)
    		return;

  	if(node->stype == S_ARRAY) 
  	{
    		size = typesize(value_at(node->type), node->ctype);
    		type = value_at(node->type);
  	} 
  	else 
  	{
    		size = node->size;
    		type = node->type;
  	}

  	cgdataseg();
  	if(node->class == C_GLOBAL)
    		fprintf(Outfile, "export ");
  	if((node->type == P_STRUCT) || (node->type == P_UNION))
    		fprintf(Outfile, "data $%s = align 8 { ", node->name);
  	else
    		fprintf(Outfile, "data $%s = align %d { ", node->name, cgprimsize(type));

  	for(i = 0; i < node->nelems; i++) 
  	{
		initvalue = 0;
    		if(node->initlist != NULL)
      			initvalue = node->initlist[i];

    		switch(size) 
    		{
      			case 1:
				fprintf(Outfile, "b %d, ", initvalue);
				break;
      			case 4:
				fprintf(Outfile, "w %d, ", initvalue);
				break;
      			case 8:
				if(node->initlist != NULL && type == pointer_to(P_CHAR) && initvalue != 0)
	  				fprintf(Outfile, "l $L%d, ", initvalue);
				else
	  				fprintf(Outfile, "l %d, ", initvalue);
				break;	
      			default:
				fprintf(Outfile, "z %d, ", size);
    		}
  	}
  	fprintf(Outfile, "}\n");
}

void cgglobstr(int l, char *strvalue, int append) 
{
  	char *cptr;
  	if(!append)
    		fprintf(Outfile, "data $L%d = { ", l);

  	for(cptr = strvalue; *cptr; cptr++) 
  	{
    		fprintf(Outfile, "b %d, ", *cptr);
  	}
}

void cgglobstrend(void) 
{
  	fprintf(Outfile, " b 0 }\n");
}

static char *cmplist[] = { "ceq", "cne", "cslt", "csgt", "csle", "csge" };

int cgcompare_and_set(int ASTop, int r1, int r2, int type) 
{
  	int r3;
  	char q = cgqbetype(type);

  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	r3 = cgalloctemp();

  	fprintf(Outfile, "  %%.t%d =%c %s%c %%.t%d, %%.t%d\n", r3, q, cmplist[ASTop - A_EQ], q, r1, r2);
  	
  	return r3;
}

void cglabel(int l) 
{
  	fprintf(Outfile, "@L%d\n", l);
}

void cgjump(int l) 
{
  	fprintf(Outfile, "  jmp @L%d\n", l);
}

static char *invcmplist[] = { "cne", "ceq", "csge", "csle", "csgt", "cslt" };

int cgcompare_and_jump(int ASTop, int r1, int r2, int label, int type) 
{
  	int label2;
  	int r3;
  	char q = cgqbetype(type);

  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	label2 = genlabel();

  	r3 = cgalloctemp();

  	fprintf(Outfile, "  %%.t%d =%c %s%c %%.t%d, %%.t%d\n", r3, q, invcmplist[ASTop - A_EQ], q, r1, r2);
  	fprintf(Outfile, "  jnz %%.t%d, @L%d, @L%d\n", r3, label, label2);
  	cglabel(label2);
  	
  	return NOREG;
}

int cgwiden(int r, int oldtype, int newtype) 
{
  	char oldq = cgqbetype(oldtype);
  	char newq = cgqbetype(newtype);

  	int t = cgalloctemp();

  	switch(oldtype) 
  	{
    		case P_CHAR:
      			fprintf(Outfile, "  %%.t%d =%c extub %%.t%d\n", t, newq, r);
      			break;
    		default:
      			fprintf(Outfile, "  %%.t%d =%c exts%c %%.t%d\n", t, newq, oldq, r);
  	}
  	return t;
}

void cgreturn(int reg, symt *sym) 
{
  	if(reg != NOREG)
    		fprintf(Outfile, "  %%.ret =%c copy %%.t%d\n", cgqbetype(sym->type), reg);

  	cgjump(sym->st_endlabel);
}

int cgaddress(symt *sym) 
{
  	int r = cgalloctemp();
  	char qbeprefix = ((sym->class == C_GLOBAL) || (sym->class == C_STATIC) || 
  				(sym->class == C_EXTERN)) ? '$' : '%';

  	fprintf(Outfile, "  %%.t%d =l copy %c%s\n", r, qbeprefix, sym->name);
  	
  	return r;
}

int cgderef(int r, int type) 
{
  	int newtype = value_at(type);

  	int size = cgprimsize(newtype);

  	int ret = cgalloctemp();

  	switch(size) 
  	{
    		case 1:
      			fprintf(Outfile, "  %%.t%d =w loadub %%.t%d\n", ret, r);
      			break;
    		case 4:
      			fprintf(Outfile, "  %%.t%d =w loadsw %%.t%d\n", ret, r);
      			break;
    		case 8:
      			fprintf(Outfile, "  %%.t%d =l loadl %%.t%d\n", ret, r);
      			break;
    		default:
      			fatald("Can't cgderef on type:", type);
  	}
  	return ret;
}

int cgstorderef(int r1, int r2, int type) 
{
  	int size = cgprimsize(type);

  	switch(size) 
  	{
    		case 1:
      			fprintf(Outfile, "  storeb %%.t%d, %%.t%d\n", r1, r2);
      			break;
    		case 4:
      			fprintf(Outfile, "  storew %%.t%d, %%.t%d\n", r1, r2);
      			break;
    		case 8:
      			fprintf(Outfile, "  storel %%.t%d, %%.t%d\n", r1, r2);
      			break;
    		default:
      			fatald("Can't cgstoderef on type:", type);
  	}
  	return r1;
}

void cgmove(int r1, int r2, int type) 
{
  	fprintf(Outfile, "  %%.t%d =%c copy %%.t%d\n", r2, cgqbetype(type), r1);
}

void cglinenum(int line) 
{
}

int cgcast(int t, int oldtype, int newtype) 
{
  	int ret = cgalloctemp();
  	int oldsize, newsize;
  	char qnew;

  	if(ptrtype(newtype)) 
  	{
    		if(ptrtype(oldtype))
      			return t;

    		return (cgwiden(t, oldtype, newtype));
  	}

  	qnew = cgqbetype(newtype);
  	oldsize = cgprimsize(oldtype);
  	newsize = cgprimsize(newtype);

  	if(newsize == oldsize)
    		return t;

  	if(newsize < oldsize)
    		fprintf(Outfile, " %%.t%d =%c copy %%.t%d\n", ret, qnew, t);
  	else
    		fprintf(Outfile, " %%.t%d =%c cast %%.t%d\n", ret, qnew, t);
  	return ret;
}
