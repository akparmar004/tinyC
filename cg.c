#include "defs.h"
#include "data.h"
#include "decl.h"

//assembly code generator for x86-64

//flag to say that in which section we are giving output into
enum { no_seg, text_seg, data_seg } currSeg = no_seg;

void cgtextseg()
{
	if(currSeg != text_seg)
	{
		fputs("\t.text\n",Outfile);
		currSeg = text_seg;
	}
}

void cgdataseg() 
{
  	if(currSeg != data_seg) 
	{
    		fputs("\t.data\n", Outfile);
    		currSeg = data_seg;
  	}
}

//position of next local variable relative to stack base pointer.
//we store the offset as positive to make aligning the stack pointer easier
static int localOffset;
static int stackOffset;

//create the position of a new local variable.
static int newlocaloffset(int type) 
{
  	//decrement the offset by a minimum of 4 bytes and allocate on the stack
  	localOffset += (cgprimsize(type) > 4) ? cgprimsize(type) : 4;
  	
	return -localOffset;
}

//list of available registers and their names, we need a list of byte and doubleword registers, too
//the list also includes the registers used to hold function parameters
#define NUMFREEREGS 4
#define FIRSTPARAMREG 9		//position of first parameter register

static int freereg[NUMFREEREGS];

static char *reglist[] =  {"%r10", "%r11", "%r12", "%r13", "%r9", "%r8", "%rcx", "%rdx", "%rsi", "%rdi"};
static char *dreglist[] = {"%r10d", "%r11d", "%r12d", "%r13d", "%r9d", "%r8d", "%ecx", "%edx", "%esi", "%edi"};
static char *breglist[] = {"%r10b", "%r11b", "%r12b", "%r13b", "%r9b", "%r8b", "%cl", "%dl", "%sil", "%dil"};


//set all registers as free regis...
void freeall_registers(void) 
{
	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

//allocate a free register. Return the number of the register, if there is no regis.. available then simply 
//returns NOREG
static int alloc_register(void) 
{
  	for (int i = 0; i < NUMFREEREGS; i++) 
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
}

//nothing to do
void cgpostamble() 
{
}

//function preamble
void cgfuncpreamble(symt *sym) 
{
	char *name = sym -> name;
  	struct symtable *parm, *locvar;
  	int cnt;
  	int paramOffset = 16;		//any pushed params start at this stack offset
  	int paramReg = FIRSTPARAMREG;	//index to the first param register in above reg lists

  	//output in the text segment, reset local offset
  	cgtextseg();
  	localOffset = 0;

  	//output the function start, save the %rsp and %rsp
  	fprintf(Outfile,
	  	"\t.globl\t%s\n"
	  	"\t.type\t%s, @function\n"
	  	"%s:\n" "\tpushq\t%%rbp\n"
	  	"\tmovq\t%%rsp, %%rbp\n", name, name, name);

  	//copy any in-register parameters to the stack, up to six of them
  	//the remaining parameters are already on the stack
  	for(parm = sym->member, cnt = 1; parm != NULL; parm = parm->next, cnt++) 
	{
    		if(cnt > 6) 
		{
      			parm -> posn = paramOffset;
      			paramOffset += 8;
    		} 
		else 
		{
      			parm -> posn = newlocaloffset(parm -> type);
      			cgstorlocal(paramReg--, parm);
    		}
  	}

  	//for the remainder, if they are a parameter then they are 
	//already on the stack. If only a local, make a stack position.
  	for(locvar = Loclhead; locvar != NULL; locvar = locvar->next) 
	{
    		locvar->posn = newlocaloffset(locvar->type);
  	}

  	//align the stack pointer to be a multiple of 16 less than its previous value
  	stackOffset = (localOffset + 15) & ~15;
  	fprintf(Outfile, "\taddq\t$%d,%%rsp\n", -stackOffset);
}

//function postamble
void cgfuncpostamble(symt *sym) 
{
  	cglabel(sym -> endlabel);
  	fprintf(Outfile, "\taddq\t$%d,%%rsp\n", stackOffset);
	fputs("\tpopq	%rbp\n" "\tret\n", Outfile);
}

//load int lit value into a regis.. and  return the number of the regis...
int cgloadint(int value, int type) 
{
  	//first get new register
  	int r = alloc_register();

	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
  	return r;
}

//load a value from a variable into a regis..,  return the number of the register
int cgloadglob(symt *sym, int op) 
{
	//get a new register
  	int r = alloc_register();

  	if(cgprimsize(sym -> type) == 8) 
	{
    		if(op == A_PREINC)
      			fprintf(Outfile, "\tincq\t%s(%%rip)\n", sym->name);
    	
		if(op == A_PREDEC)
      			fprintf(Outfile, "\tdecq\t%s(%%rip)\n", sym->name);
    		
		fprintf(Outfile, "\tmovq\t%s(%%rip), %s\n", sym->name, reglist[r]);
    		if(op == A_POSTINC)
      			fprintf(Outfile, "\tincq\t%s(%%rip)\n", sym->name);
    		
		if(op == A_POSTDEC)
      			fprintf(Outfile, "\tdecq\t%s(%%rip)\n", sym->name);
  	} 
	else
    		//print out the code to initialise it
    		switch(sym -> type) 
		{
      			case P_CHAR:
				if(op == A_PREINC)
	  				fprintf(Outfile, "\tincb\t%s(%%rip)\n", sym->name);
				
				if(op == A_PREDEC)
	  				fprintf(Outfile, "\tdecb\t%s(%%rip)\n", sym->name);
				
				fprintf(Outfile, "\tmovzbq\t%s(%%rip), %s\n", sym->name, reglist[r]);
				if(op == A_POSTINC)
	  				fprintf(Outfile, "\tincb\t%s(%%rip)\n", sym->name);
				
				if(op == A_POSTDEC)
	  				fprintf(Outfile, "\tdecb\t%s(%%rip)\n", sym->name);
				break;
      			case P_INT:
				if(op == A_PREINC)
	  				fprintf(Outfile, "\tincl\t%s(%%rip)\n", sym->name);
				
				if(op == A_PREDEC)
	  				fprintf(Outfile, "\tdecl\t%s(%%rip)\n", sym->name);
				
				fprintf(Outfile, "\tmovslq\t%s(%%rip), %s\n", sym->name, reglist[r]);
				if(op == A_POSTINC)
	  				fprintf(Outfile, "\tincl\t%s(%%rip)\n", sym->name);
				
				if(op == A_POSTDEC)
	  				fprintf(Outfile, "\tdecl\t%s(%%rip)\n", sym->name);
				break;
      			default:
				fatald("Bad type in cgloadglob:", sym->type);
    		}
  	return r;
}

// Load a value from a local variable into a register.
// Return the number of the register. If the
// operation is pre- or post-increment/decrement,
// also perform this action.
int cgloadlocal(struct symtable *sym, int op) 
{
  	// Get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	if(cgprimsize(sym->type) == 8) 
	{
    		if(op == A_PREINC)
      			fprintf(Outfile, "\tincq\t%d(%%rbp)\n", sym->posn);
    		
		if(op == A_PREDEC)
      			fprintf(Outfile, "\tdecq\t%d(%%rbp)\n", sym->posn);
    		
		fprintf(Outfile, "\tmovq\t%d(%%rbp), %s\n", sym->posn, reglist[r]);
    		if(op == A_POSTINC)
      			fprintf(Outfile, "\tincq\t%d(%%rbp)\n", sym->posn);
    		if(op == A_POSTDEC)
      			fprintf(Outfile, "\tdecq\t%d(%%rbp)\n", sym->posn);
  	} 
	else
    		switch (sym -> type) 
		{
      			case P_CHAR:
				if(op == A_PREINC)
	  				fprintf(Outfile, "\tincb\t%d(%%rbp)\n", sym->posn);
				
				if(op == A_PREDEC)
	  				fprintf(Outfile, "\tdecb\t%d(%%rbp)\n", sym->posn);
				
				fprintf(Outfile, "\tmovzbq\t%d(%%rbp), %s\n", sym->posn, reglist[r]);
				if(op == A_POSTINC)
	  				fprintf(Outfile, "\tincb\t%d(%%rbp)\n", sym->posn);
				
				if(op == A_POSTDEC)
	  				fprintf(Outfile, "\tdecb\t%d(%%rbp)\n", sym->posn);
				break;
      			case P_INT:
				if(op == A_PREINC)
	  				fprintf(Outfile, "\tincl\t%d(%%rbp)\n", sym->posn);
				
				if(op == A_PREDEC)
	  				fprintf(Outfile, "\tdecl\t%d(%%rbp)\n", sym->posn);
				
				fprintf(Outfile, "\tmovslq\t%d(%%rbp), %s\n", sym->posn, reglist[r]);
				if(op == A_POSTINC)
	  				fprintf(Outfile, "\tincl\t%d(%%rbp)\n", sym->posn);
				
				if(op == A_POSTDEC)
	  				fprintf(Outfile, "\tdecl\t%d(%%rbp)\n", sym->posn);
				break;
      			default:
				fatald("Bad type in cgloadlocal:", sym->type);
    		}
  	return r;
}


int cgloadglobstr(int label)
{
	int r = alloc_register();
	fprintf(Outfile, "\tleaq\tL%d(\%%rip), %s\n", label, reglist[r]);

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

int cgand(int r1, int r2) 
{
  	fprintf(Outfile, "\tandq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	
	return r2;
}

int cgor(int r1, int r2) 
{
  	fprintf(Outfile, "\torq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	
	return r2;
}

int cgxor(int r1, int r2) 
{
  	fprintf(Outfile, "\txorq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

int cgshl(int r1, int r2) 
{
  	fprintf(Outfile, "\tmovb\t%s, %%cl\n", breglist[r2]);
  	fprintf(Outfile, "\tshlq\t%%cl, %s\n", reglist[r1]);
  	free_register(r2);
  
	return r1;
}

int cgshr(int r1, int r2) 
{
  	fprintf(Outfile, "\tmovb\t%s, %%cl\n", breglist[r2]);
  	fprintf(Outfile, "\tshrq\t%%cl, %s\n", reglist[r1]);
  	free_register(r2);
  	return r1;
}

//negate a register's value
int cgnegate(int r) 
{
  	fprintf(Outfile, "\tnegq\t%s\n", reglist[r]);
  	return r;
}

//invert a register's value
int cginvert(int r) 
{
  	fprintf(Outfile, "\tnotq\t%s\n", reglist[r]);
  	return r;
}

//logically negate a register's value
int cglognot(int r) 
{
  	fprintf(Outfile, "\ttest\t%s, %s\n", reglist[r], reglist[r]);
  	fprintf(Outfile, "\tsete\t%s\n", breglist[r]);
  	fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);
  	
	return r;
}

//convert an integer value to a boolean value, jump if it's an IF or WHILE operation
int cgboolean(int r, int op, int label) 
{
  	fprintf(Outfile, "\ttest\t%s, %s\n", reglist[r], reglist[r]);
  	if(op == A_IF || op == A_WHILE)
    		fprintf(Outfile, "\tje\tL%d\n", label);
  	else 
	{
    		fprintf(Outfile, "\tsetnz\t%s\n", breglist[r]);
    		fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);
  	}
  	return r;
}

//call a function with one argument from the given register return the regis..
int cgcall(symt *sym, int numargs) 
{
	 //get a new register
  	int outr = alloc_register();
  	
	//call the function
  	fprintf(Outfile, "\tcall\t%s@PLT\n", sym->name);
  
	//remove any arguments pushed on the stack
  	if(numargs > 6)
    		fprintf(Outfile, "\taddq\t$%d, %%rsp\n", 8 * (numargs - 6));
  	
	//and copy the return value into our register
  	fprintf(Outfile, "\tmovq\t%%rax, %s\n", reglist[outr]);
  	return outr;
}

// Given a register with an argument value,
// copy this argument into the argposn'th
// parameter in preparation for a future function
// call. Note that argposn is 1, 2, 3, 4, ..., never zero.
void cgcopyarg(int r, int argposn) 
{
  	// If this is above the sixth argument, simply push the
  	// register on the stack. We rely on being called with
  	// successive arguments in the correct order for x86-64
  	if(argposn > 6) 
	{
    		fprintf(Outfile, "\tpushq\t%s\n", reglist[r]);
  	} 
	else 
	{
    		// Otherwise, copy the value into one of the six registers used to hold parameter values
    		fprintf(Outfile, "\tmovq\t%s, %s\n", reglist[r],
	    	reglist[FIRSTPARAMREG - argposn + 1]);
  	}
}

//shift a regis. left by a constant
int cgshlconst(int r, int val) 
{
  	fprintf(Outfile, "\tsalq\t$%d, %s\n", val, reglist[r]);
  	return r;
}

//store a register's value into a variable
int cgstorglob(int r, struct symtable *sym) 
{
  	if(cgprimsize(sym->type) == 8) 
	{
    		fprintf(Outfile, "\tmovq\t%s, %s(%%rip)\n", reglist[r], sym->name);
  	} 
	else
    		switch(sym -> type) 
		{
      			case P_CHAR:
				fprintf(Outfile, "\tmovb\t%s, %s(%%rip)\n", breglist[r], sym->name);
				break;
      			case P_INT:
				fprintf(Outfile, "\tmovl\t%s, %s(%%rip)\n", dreglist[r], sym->name);
				break;
      			default:
				fatald("Bad type in cgstorglob:", sym->type);
    		}
  	return r;
}

//store a register's value into a local variable
int cgstorlocal(int r, struct symtable *sym) 
{
  	if(cgprimsize(sym->type) == 8) 
	{
    		fprintf(Outfile, "\tmovq\t%s, %d(%%rbp)\n", reglist[r], sym->posn);
  	} 
	else
    		switch (sym -> type) 
		{
      			case P_CHAR:
				fprintf(Outfile, "\tmovb\t%s, %d(%%rbp)\n", breglist[r], sym->posn);
				break;
      			case P_INT:
				fprintf(Outfile, "\tmovl\t%s, %d(%%rbp)\n", dreglist[r], sym->posn);
				break;
      			default:
				fatald("Bad type in cgstorlocal:", sym->type);
    		}
  	return r;
}

//return size of primitive type in byte
int cgprimsize(int type) 
{
	if(ptrtype(type))
    		return 8;
  	
	switch(type) 
	{
    		case P_CHAR:
      			return 1;
    		case P_INT:
      			return 4;
    		case P_LONG:
      			return 8;
    		default:
      			fatald("Bad type in cgprimsize:", type);
  	}
  	return 0;
}

//generate a global symbol
void cgglobsym(symt *node) 
{
	int typesize;

  	if(node == NULL)
    		return;
  	if(node -> stype == S_FUNCTION)
    		return;

  	//get the size of the type
  	typesize = cgprimsize(node->type);

  	//generate the global identity and the label
  	cgdataseg();
  	fprintf(Outfile, "\t.globl\t%s\n", node->name);
  	fprintf(Outfile, "%s:", node->name);

  	//generate the space
  	for(int i = 0; i < node->size; i++) 
	{
    		switch(typesize) 
		{
      			case 1:
				fprintf(Outfile, "\t.byte\t0\n");
				break;
      			case 4:
				fprintf(Outfile, "\t.long\t0\n");
				break;
      			case 8:
				fprintf(Outfile, "\t.quad\t0\n");
				break;
      			default:
				fatald("Unknown typesize in cgglobsym: ", typesize);
    		}
  	}
}

void cgglobstr(int l, char *strvalue)
{
	char *cptr;
	cglabel(l);

	for(cptr = strvalue; *cptr; cptr++)
	{
		fprintf(Outfile, "\t.byte\t%d\n", *cptr);
	}

	fprintf(Outfile,"\t.byte\t0\n");
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
void cgreturn(int reg, symt *sym) 
{
  	//gen code depending on the function's type
  	switch(sym -> type) 
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
      			fatald("Bad function type in cgreturn:", sym -> type);
  	}
  	cgjump(sym -> endlabel);
}


//code to load the address of a global ident into a var, return a new regis..
int cgaddress(symt *sym)
{
  	int r = alloc_register();
	
	if(sym -> class == C_GLOBAL)
    		fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", sym -> name, reglist[r]);
  	else
    		fprintf(Outfile, "\tleaq\t%d(%%rbp), %s\n", sym -> posn, reglist[r]);
  	
	return r;
}

//dereference a pointer to get the value it pointing at into regis..
int cgderef(int r, int type) 
{
	//get the type that we are pointing to
  	int newtype = value_at(type);
  	
	//now get the size of this type
  	int size = cgprimsize(newtype);
  	
	switch(size) 
	{
		case 1:
      			fprintf(Outfile, "\tmovzbq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		case 2:
      			fprintf(Outfile, "\tmovslq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		case 4:
    		case 8:
      			fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		default:
      			fatald("Can't cgderef on type:", type);
  	}
  	return r;
}

//store through a dereferenced pointer
int cgstorderef(int r1, int r2, int type) 
{
	int size = cgprimsize(type);

  	switch(size) 
  	{
		case 1:
      			fprintf(Outfile, "\tmovb\t%s, (%s)\n", breglist[r1], reglist[r2]);
      			break;
    		case 2:
    		case 4:
    		case 8:
      			fprintf(Outfile, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
      			break;
    		default:
      			fatald("Can't cgstoderef on type:", type);
  	}
  	return r1;
}
