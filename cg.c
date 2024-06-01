#include "defs.h"
#include "data.h"
#include "decl.h"

//assembly code generator for x86-64

//list of available registers with their names, general purpose registers..
//and also we need a list of byte and doubleword registers... 
static int freereg[4];
static char *reglist[4] = { "%r8", "%r9", "%r10", "%r11" };
static char *dreglist[4] = { "%r8d", "%r9d", "%r10d", "%r11d" };
static char *breglist[4] = { "%r8b", "%r9b", "%r10b", "%r11b" };


//set all registers as free regis...
void freeall_registers(void) 
{
	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

//allocate a free register. Return the number of the register, if there is no regis.. available then simply 
//returns NOREG
static int alloc_register(void) 
{
  	for (int i = 0; i < 4; i++) 
	{
    		if(freereg[i]) 
		{
      			freereg[i] = 0;
      			return i;
    		}
  	}
  	fatal("Out of registers");
  	return NOREG;
}

//set a particular register as free with given reg id, check to see if it's not already there.
static void free_register(int reg) 
{
  	if(freereg[reg] != 0)
    		fatald("Error trying to free register", reg);
  	
	freereg[reg] = 1;
}

//print assembly preamble which is initial setup code for assem. file..
void cgpreamble() 
{
  	freeall_registers();
  	fputs("\t.text\n", Outfile);
}

//nothing to do
void cgpostamble() 
{
}

//function preamble
void cgfuncpreamble(int id) 
{
  	char *name = Gsym[id].name;
  	fprintf(Outfile,
	  		"\t.text\n"
			"\t.globl\t%s\n"
	    	        "\t.type\t%s, @function\n"
	                "%s:\n" "\tpushq\t%%rbp\n"
	                "\tmovq\t%%rsp, %%rbp\n", name, name, name);
}

//function postamble
void cgfuncpostamble(int id) 
{
  	cglabel(Gsym[id].endlabel);
  	fputs("\tpopq	%rbp\n" "\tret\n", Outfile);
}

//load int lit value into a regis.. and  return the number of the regis...
//for x86-64, we don't need to worry about the type.
int cgloadint(int value, int type) 
{
  	//first get new register
  	int r = alloc_register();

	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
  	return r;
}

//load a value from a variable into a regis..,  return the number of the register
int cgloadglob(int id) 
{
  	//get new register
  	int r = alloc_register();

  	//print the code to initialise it
  	switch(Gsym[id].type) 
	{
    		case P_CHAR:
      			fprintf(Outfile, "\tmovzbq\t%s(%%rip), %s\n", Gsym[id].name, reglist[r]);
      			break;
    		case P_INT:
      			fprintf(Outfile, "\tmovzbl\t%s(\%%rip), %s\n", Gsym[id].name, reglist[r]);
      			break;
    		
		case P_LONG:
    		case P_CHARPTR:
    		case P_INTPTR:
    		case P_LONGPTR:
      			fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", Gsym[id].name, reglist[r]);
      			break;
    		default:
      			fatald("Bad type in cgloadglob:", Gsym[id].type);
  	}
  	return r;
}

//add two registers together and return the number of the register which having the result
int cgadd(int r1, int r2) 
{
	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

//subtract the second register from the first and return the number of the regis..
int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r2);
  	return r1;
}

//multiply two regis.. and return the number of the regis..
int cgmul(int r1, int r2) 
{
  	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

//divide the first register by the second and return the number of the regis..
int cgdiv(int r1, int r2) 
{
  	fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
  	fprintf(Outfile, "\tcqo\n");
  	fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
  	fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
  	free_register(r2);
  	return r1;
}

//call a function with one argument from the given register return the regis..
int cgcall(int r, int id) 
{
  	//allocate new register
  	int outr = alloc_register();
  	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
  	fprintf(Outfile, "\tcall\t%s\n", Gsym[id].name);
  	fprintf(Outfile, "\tmovq\t%%rax, %s\n", reglist[outr]);
  	free_register(r);
  	return outr;
}

//shift a regis. left by a constant
int cgshlconst(int r, int val) 
{
  	fprintf(Outfile, "\tsalq\t$%d, %s\n", val, reglist[r]);
  	return r;
}

//store a register's value into a variable
int cgstorglob(int r, int id) 
{
  	switch (Gsym[id].type) 
	{
    		case P_CHAR:
      			fprintf(Outfile, "\tmovb\t%s, %s(\%%rip)\n", breglist[r], Gsym[id].name);
      			break;
    		case P_INT:
      			fprintf(Outfile, "\tmovl\t%s, %s(\%%rip)\n", dreglist[r], Gsym[id].name);
      			break;
    		
		case P_LONG:
    		case P_CHARPTR:
    		case P_INTPTR:
    		case P_LONGPTR:
      			fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], Gsym[id].name);
      			break;
    		default:
      			fatald("Bad type in cgloadglob:", Gsym[id].type);
  	}
  	return r;
}

//list of type sizes in P_XXX order and 0 means no size.
static int psize[] = { 0, 0, 1, 4, 8, 8, 8, 8, 8 };

//return size of primitive type in byte
int cgprimsize(int type) 
{
  	//check the type is valid
  	if(type < P_NONE || type > P_LONGPTR)
    		fatal("wrong type in cgprimsize()");
  	return (psize[type]);
}

//generate a global symbol
void cgglobsym(int id) 
{
  	int typesize;
  	
	//get the size of type
  	typesize = cgprimsize(Gsym[id].type);

  	//generating global identity and the label
  	fprintf(Outfile, "\t.data\n" "\t.globl\t%s\n", Gsym[id].name);
  	fprintf(Outfile, "%s:", Gsym[id].name);

  	//initialise the space for data
  	for(int i=0; i < Gsym[id].size; i++) 
	{
    		switch(typesize) 
		{
      			case 1: fprintf(Outfile, "\t.byte\t0\n"); 
				break;
      			case 4: fprintf(Outfile, "\t.long\t0\n"); 
				break;
      			case 8: fprintf(Outfile, "\t.quad\t0\n"); 
				break;
      			default: fatald("Unknown typesize in cgglobsym: ", typesize);
    		}
  	}
}

//list of comparison instructions, 
         //in AST order:    A_EQ,   A_NE,    A_LT,   A_GT,   A_LE,    A_GE
static char *cmplist[] = { "sete", "setne", "setl", "setg", "setle", "setge" };

//compare two registers and set if true.
int cgcompare_and_set(int ASTop, int r1, int r2) 
{
	//chek for range of AST operation if it not in there then just ret. err..
  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  	fprintf(Outfile, "\t%s\t%s\n", cmplist[ASTop - A_EQ], breglist[r2]);
  	fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);
  	free_register(r1);
  	return r2;
}

//generate a label
void cglabel(int l) 
{
  	fprintf(Outfile, "L%d:\n", l);
}

//generatee a jump to a label
void cgjump(int l) 
{
  	fprintf(Outfile, "\tjmp\tL%d\n", l);
}

// List of inverted jump instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *invcmplist[] = { "jne", "je", "jge", "jle", "jg", "jl" };

//compare two registers and jump if false...
int cgcompare_and_jump(int ASTop, int r1, int r2, int label) 
{
  	//check for range of the AST operation
  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  	fprintf(Outfile, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], label);
  	freeall_registers();
  	return NOREG;
}

//widen the value in the register from the old to the new type, and return a register with
//this new value
int cgwiden(int r, int oldtype, int newtype) 
{
  	return r;
}

//gen code to return a value from a function
void cgreturn(int reg, int id) 
{
  	//gen code depending on the function's type
  	switch(Gsym[id].type) 
	{
    		case P_CHAR:
      			fprintf(Outfile, "\tmovzbl\t%s, %%eax\n", breglist[reg]);
      			break;
    		case P_INT:
      			fprintf(Outfile, "\tmovl\t%s, %%eax\n", dreglist[reg]);
      			break;
    		case P_LONG:
      			fprintf(Outfile, "\tmovq\t%s, %%rax\n", reglist[reg]);
      			break;
    		default:
      			fatald("Bad function type in cgreturn:", Gsym[id].type);
  	}
  	cgjump(Gsym[id].endlabel);
}


//write instruction to load the address of a global ident into a var, return a new regis..
int cgaddress(int id)
{
  	int r = alloc_register();

  	fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", Gsym[id].name, reglist[r]);
  	return r;
}

//dereference a pointer to get the value it pointing at into the same register
int cgderef(int r, int type) 
{
  	switch (type) 
	{
    		case P_CHARPTR:
      			fprintf(Outfile, "\tmovzbq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		case P_INTPTR:
      			fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		case P_LONGPTR:
      			fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		default:
      			fatald("Can't cgderef on type:", type);
  	}
  	return r;
}

// Store through a dereferenced pointer
int cgstorderef(int r1, int r2, int type) {
  	switch (type) 
  	{
    		case P_CHAR:
      			fprintf(Outfile, "\tmovb\t%s, (%s)\n", breglist[r1], reglist[r2]);
      			break;
    		case P_INT:
      			fprintf(Outfile, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
      			break;
    		case P_LONG:
      			fprintf(Outfile, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
      			break;
    		default:
      		fatald("Can't cgstoderef on type:", type);
  	}
  	return r1;
}
