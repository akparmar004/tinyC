#include<defs.h>
#include<data.h>
#include<decl.h>

//code generator for armv6

static int freereg[4];
static char *reglist = {"r4", "r5", "r6", "r7"};

void freeall_register(void)
{
	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1; 
}

static int alloc_register(void)
{
	for(int i = 0;i < 4; i++)
	{
		if(freereg[i])
		{
			freereg[i] = 0;
			return i;
		}
	}
	fatal("Out of register..");
	return NOREG;
}

static void free_register(int reg)
{
	if(freereg[reg] != 0)
		fatald("Error trying to free the register..", reg);
		
	freereg[reg] = 1;
}

#define MAXINTS 1024
int Intlist[MAXINTS];
static int Intslot = 0;

static void set_int_offset(int val) 
{
  	int offset = -1;

  	for(int i = 0; i < Intslot; i++) 
  	{
    		if(Intlist[i] == val) 
    		{
      			offset = 4 * i;
      			break;
    		}
  	}
	
	if(offset == -1)
	{
		offset = 4 * Intslot;
		if(Intslot == MAXINTS)
			fatal("Out of int slots in set_int_offset()");
			
		Intlist[Intslot++] = val;
	}
	fprintf(Outfile, "\tldr\tr3, .L3+%d\n", offset); 
}

void cgpreamble()
{
	freeall_register();
	fputs("\t.text\n", Outfile);
}

void cgpostamble()
{
	fprintf(Outfile, ".L2:\n");
	for(int i = 0;i < Globs; i++)
	{
		if(Symtable[i].stype == S_VARIABLE)
			fprintf(Outfile, "\t.word %s\n", Symtable[i].name);
	}
	
	fprintf(Outfile, ".L3:\n");
	for(int i = 0; i < Intslot; i++)
	{
		fprintf(Outfile, "\t.word %d\n", Intlist[i]);
	}
}

void cgfuncpreamble(int id)
{
	char *name = Symtable[id].name;
	fprintf(Outfile,
	  	"\t.text\n"
	  	"\t.globl\t%s\n"
  		"\t.type\t%s, \%%function\n"
	  	"%s:\n" "\tpush\t{fp, lr}\n"
	  	"\tadd\tfp, sp, #4\n"
	  	"\tsub\tsp, sp, #8\n" "\tstr\tr0, [fp, #-8]\n", name, name, name);
}

void funcpostamble(int id)
{
	cglabel(Symtable[id].endlabel);
  	fputs("\tsub\tsp, fp, #4\n" "\tpop\t{fp, pc}\n" "\t.align\t2\n", Outfile);
}

static void set_var_offset(int id) 
{
  	int offset = 0;

  	for(int i = 0; i < id; i++) 
  	{
    		if(Symtable[i].stype == S_VARIABLE)
      			offset += 4;
  	}
  	fprintf(Outfile, "\tldr\tr3, .L2+%d\n", offset);
}

int cgloadint(int value, int type)
{
	int r = alloc_register();
	
	if(value <= 1000)
	{
		fprintf(Outfile, "\tmov\t%s, #%d\n", reglist[r], value);
	}
	else
	{
		set_int_offset(value);
    		fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
	}
	return r;
}

int cgloadglob(int id) 
{
  	int r = alloc_register();

  	set_var_offset(id);

  	switch(Symtable[id].type) 
  	{
    		case P_CHAR:
      			fprintf(Outfile, "\tldrb\t%s, [r3]\n", reglist[r]);
      			break;
    		case P_INT:
    		case P_LONG:
    		case P_CHARPTR:
    		case P_INTPTR:
    		case P_LONGPTR:
      			fprintf(Outfile, "\tldr\t%s, [r3]\n", reglist[r]);
      			break;
    		default:
      			fatald("Bad type in cgloadglob:", Symtable[id].type);
  	}
  	return r;
}

int cgadd(int r1, int r2) 
{
  	fprintf(Outfile, "\tadd\t%s, %s, %s\n", reglist[r2], reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsub\t%s, %s, %s\n", reglist[r1], reglist[r1], reglist[r2]);
  	free_register(r2);
  	return r1;
}

int cgmul(int r1, int r2) 
{
  	fprintf(Outfile, "\tmul\t%s, %s, %s\n", reglist[r2], reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

int cgdiv(int r1, int r2) 
{
  	fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r1]);
  	fprintf(Outfile, "\tmov\tr1, %s\n", reglist[r2]);
  	fprintf(Outfile, "\tbl\t__aeabi_idiv\n");
  	fprintf(Outfile, "\tmov\t%s, r0\n", reglist[r1]);
  	free_register(r2);
  	return r1;
}

int cgcall(int r, int id) 
{
  	fprintf(Outfile, "\tmov\tr0, %s\n", reglist[r]);
  	fprintf(Outfile, "\tbl\t%s\n", Symtable[id].name);
  	fprintf(Outfile, "\tmov\t%s, r0\n", reglist[r]);
  	return r;
}

int cgshlconst(int r, int val) 
{
  	fprintf(Outfile, "\tlsl\t%s, %s, #%d\n", reglist[r], reglist[r], val);
  	return r;
}

int cgstorglob(int r, int id) 
{
  	set_var_offset(id);

  	switch(Symtable[id].type) 
  	{
    		case P_CHAR:
      			fprintf(Outfile, "\tstrb\t%s, [r3]\n", reglist[r]);
      			break;
    		case P_INT:
    		case P_LONG:
    		case P_CHARPTR:
    		case P_INTPTR:
    		case P_LONGPTR:
      			fprintf(Outfile, "\tstr\t%s, [r3]\n", reglist[r]);
      			break;
    		default:
      			fatald("Bad type in cgstorglob:", Symtable[id].type);
  	}
  	return r;
}

int cgprimsize(int type) 
{
  	if(ptrtype(type))
    		return 4;
  	
  	switch(type) 
  	{
    		case P_CHAR:
      			return 1;
    		case P_INT:
    		case P_LONG:
      			return 4;
    		default:
      			fatald("Bad type in cgprimsize:", type);
  	}
  	return 0;	
}

void cgglobsym(int id) 
{
  	int typesize;
  	typesize = cgprimsize(Symtable[id].type);

  	fprintf(Outfile, "\t.data\n" "\t.globl\t%s\n", Symtable[id].name);
  	switch(typesize)
  	{
    		case 1:
      			fprintf(Outfile, "%s:\t.byte\t0\n", Symtable[id].name);
      			break;
    		case 4:
      			fprintf(Outfile, "%s:\t.long\t0\n", Symtable[id].name);
      			break;
    		default:
      			fatald("Unknown typesize in cgglobsym: ", typesize);
  	}
}

static char *cmplist[] = { "moveq", "movne", "movlt", "movgt", "movle", "movge" };

static char *invcmplist[] = { "movne", "moveq", "movge", "movle", "movgt", "movlt" };

int cgcompare_and_set(int ASTop, int r1, int r2) 
{

  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  	fprintf(Outfile, "\t%s\t%s, #1\n", cmplist[ASTop - A_EQ], reglist[r2]);
  	fprintf(Outfile, "\t%s\t%s, #0\n", invcmplist[ASTop - A_EQ], reglist[r2]);
  	fprintf(Outfile, "\tuxtb\t%s, %s\n", reglist[r2], reglist[r2]);
  	free_register(r1);
  	
  	return r2;
}

void cglabel(int l)
{
  	fprintf(Outfile, "L%d:\n", l);
}

void cgjump(int l) 
{
  	fprintf(Outfile, "\tb\tL%d\n", l);
}

static char *brlist[] = { "bne", "beq", "bge", "ble", "bgt", "blt" };

int cgcompare_and_jump(int ASTop, int r1, int r2, int label) 
{
  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  	fprintf(Outfile, "\t%s\tL%d\n", brlist[ASTop - A_EQ], label);
  	freeall_registers();
  	
  	return NOREG;
}

int cgwiden(int r, int oldtype, int newtype) 
{
  	return r;
}

void cgreturn(int reg, int id) 
{
  	fprintf(Outfile, "\tmov\tr0, %s\n", reglist[reg]);
  	cgjump(Symtable[id].endlabel);
}

int cgaddress(int id) 
{
  	int r = alloc_register();

  	set_var_offset(id);
  	fprintf(Outfile, "\tmov\t%s, r3\n", reglist[r]);
  	return r;
}

int cgderef(int r, int type) 
{
  	switch(type) 
  	{
    		case P_CHARPTR:
      			fprintf(Outfile, "\tldrb\t%s, [%s]\n", reglist[r], reglist[r]);
      			break;
    		case P_INTPTR:
    		case P_LONGPTR:
      			fprintf(Outfile, "\tldr\t%s, [%s]\n", reglist[r], reglist[r]);
      			break;
  	}
  	return r;
}

int cgstorderef(int r1, int r2, int type) 
{
  	switch(type) 
  	{
    		case P_CHAR:
      			fprintf(Outfile, "\tstrb\t%s, [%s]\n", reglist[r1], reglist[r2]);
      			break;
    		case P_INT:
    		case P_LONG:
      			fprintf(Outfile, "\tstr\t%s, [%s]\n", reglist[r1], reglist[r2]);
      			break;
    		default:
      			fatald("Can't cgstoderef on type:", type);
  	}
  	return r1;
}
