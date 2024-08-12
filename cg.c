#include "defs.h"
#include "data.h"
#include "decl.h"

//code generator for x86-64

//flag to say which section were are outputting in to
enum 
{ 
	no_seg, text_seg, data_seg 
} currSeg = no_seg;

void cgtextseg() 
{
  	if(currSeg != text_seg) 
	{
    		fputs("\t.text\n", Outfile);
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

//given scalar type value, return size of type in bytes.
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

//given scalar type, existing memory offset (which hasn't been allocated to anything yet)
//and direction (1 is up, -1 is down), calculate and return suitably aligned memory offset
//for this scalar type, this could be original offset, or it could be above/below the original
int cgalign(int type, int offset, int direction) 
{
  	int alignment;

  	//we don't need to do this on x86-64, but let's align chars on any offset and align ints/pointers
  	//on a 4-byte alignment
  	switch(type) 
	{
    		case P_CHAR:
      			return offset;
    		case P_INT:
    		case P_LONG:
      			break;
    		default:
      			if(!ptrtype(type))
				fatald("Bad type in cg_align:", type);
  	}

  	//here we have int or long. align it on 4-byte offset
  	//i put generic code here so it can be reused elsewhere...
  	alignment = 4;
  	offset = (offset + direction * (alignment - 1)) & ~(alignment - 1);
  	
	return offset;
}

//position of next local variable relative to stack base pointer, 
//we store offset as positive to make aligning stack pointer easier
static int localOffset;
static int stackOffset;

//create position of new local variable.
static int newlocaloffset(int size) 
{
  	//decrement offset by minimum of 4 bytes and allocate on stack
  	localOffset += (size > 4) ? size : 4;
  	
	return -localOffset;
}

//list of available registers and their names, we need list of byte and doubleword registers, too
//the list also includes  registers used to hold function parameters

#define NUMFREEREGS 4
#define FIRSTPARAMREG 9		//position of first parameter register

static int freereg[NUMFREEREGS];
static char *reglist[] =  {"%r10", "%r11", "%r12", "%r13", "%r9", "%r8", "%rcx", "%rdx", "%rsi", "%rdi" };

static char *breglist[] = {"%r10b", "%r11b", "%r12b", "%r13b", "%r9b", "%r8b", "%cl", "%dl", "%sil", "%dil"};

static char *dreglist[] = {"%r10d", "%r11d", "%r12d", "%r13d", "%r9d", "%r8d", "%ecx", "%edx", "%esi", "%edi"};

//push and pop register on/off the stack
static void pushreg(int r) 
{
  	fprintf(Outfile, "\tpushq\t%s\n", reglist[r]);
}

static void popreg(int r) 
{
  	fprintf(Outfile, "\tpopq\t%s\n", reglist[r]);
}


//set all registers as available, but if reg is positive, don't free that one.
void freeall_registers(int keepreg) 
{
  	int i;
  	fprintf(Outfile, "# freeing all registers\n");
  	for(i = 0; i < NUMFREEREGS; i++)
    		if(i != keepreg)
      			freereg[i] = 1;
}

//when we need to spill register, we choose following register and then cycle through 
//remaining registers,  spillreg increments continually, so we need to take modulo NUMFREEREGS on it.
static int spillreg = 0;

//allocate free register, return number of register, die if no available registers..
int alloc_register(void) 
{
  	int reg;

  	for(reg = 0; reg < NUMFREEREGS; reg++) 
	{
    		if(freereg[reg]) 
		{
      			freereg[reg] = 0;
      			fprintf(Outfile, "# allocated register %s\n", reglist[reg]);
      			return reg;
    		}
  	}

  	//we have no registers, so we must spill one
  	reg = (spillreg % NUMFREEREGS);
  	spillreg++;
  	fprintf(Outfile, "# spilling reg %s\n", reglist[reg]);
  	pushreg(reg);
  	
	return reg;
}

//return register to list of available registers, check to see if it's not already there..
static void free_register(int reg) 
{
  	if(freereg[reg] != 0) 
	{
    		fprintf(Outfile, "# error trying to free register %s\n", reglist[reg]);
    		fatald("Error trying to free register", reg);
  	}

  	//if this was spilled register, get it back
  	if(spillreg > 0) 
	{
    		spillreg--;
    		reg = (spillreg % NUMFREEREGS);
    		fprintf(Outfile, "# unspilling reg %s\n", reglist[reg]);
    		popreg(reg);
  	} 
	else
       	{
    		fprintf(Outfile, "# freeing reg %s\n", reglist[reg]);
    		freereg[reg] = 1;
  	}
}

//spill all registers on stack
void spill_all_regs(void) 
{
  	int i;

  	for(i = 0; i < NUMFREEREGS; i++)
    		pushreg(i);
}

//unspill all registers from the stack
static void unspill_all_regs(void)
{
  	int i;

	for(i = NUMFREEREGS - 1; i >= 0; i--)
    		popreg(i);
}

//print out assembly preamble
void cgpreamble() 
{
  	freeall_registers(NOREG);
  	cgtextseg();
  	fprintf(Outfile,
	  "# internal switch(expr) routine\n"
	  "# %%rsi = switch table, %%rax = expr\n"
	  "# from SubC: http://www.t3x.org/subc/\n"
	  "\n"
	  "__switch:\n"
	  "        pushq   %%rsi\n"
	  "        movq    %%rdx, %%rsi\n"
	  "        movq    %%rax, %%rbx\n"
	  "        cld\n"
	  "        lodsq\n"
	  "        movq    %%rax, %%rcx\n"
	  "__next:\n"
	  "        lodsq\n"
	  "        movq    %%rax, %%rdx\n"
	  "        lodsq\n"
	  "        cmpq    %%rdx, %%rbx\n"
	  "        jnz     __no\n"
	  "        popq    %%rsi\n"
	  "        jmp     *%%rax\n"
	  "__no:\n"
	  "        loop    __next\n"
	  "        lodsq\n"
	  "        popq    %%rsi\n" "        jmp     *%%rax\n" "\n");
}

//nothing to do with this postamble
void cgpostamble() 
{
}

//print out function preamble
void cgfuncpreamble(symt *sym) 
{
  	char *name = sym->name;
  	symt *parm, *locvar;
  	int cnt;
  	int paramOffset = 16;		//any pushed params start at this stack offset
  	int paramReg = FIRSTPARAMREG;	//index to first param register in above reg lists

  	//output in text segment, reset local offset
  	cgtextseg();
  	localOffset = 0;

  	//output function start, save %rsp and %rsp
  	if(sym->class == C_GLOBAL)
    		fprintf(Outfile, "\t.globl\t%s\n" "\t.type\t%s, @function\n", name, name);
  	
	fprintf(Outfile, "%s:\n" "\tpushq\t%%rbp\n" "\tmovq\t%%rsp, %%rbp\n", name);

  	//copy any in-register parameters to stack, up to six of them
  	//the remaining parameters are already on the stack
  	for(parm = sym->member, cnt = 1; parm != NULL; parm = parm->next, cnt++) 
	{
    		if(cnt > 6) 
		{
      			parm->st_posn = paramOffset;
      			paramOffset += 8;
    		} 
		else 
		{
      			parm -> st_posn = newlocaloffset(parm->size);
      			cgstorlocal(paramReg--, parm);
    		}
  	}

  	//for remainder, if they are parameter then they are
  	//already on stack, If only local, make stack position.
  	for(locvar = Loclhead; locvar != NULL; locvar = locvar->next) 
	{
    		locvar->st_posn = newlocaloffset(locvar->size);
  	}

  	//align stack pointer to be multiple of 16
  	//less than its previous value
  	stackOffset = (localOffset + 15) & ~15;
  	fprintf(Outfile, "\taddq\t$%d,%%rsp\n", -stackOffset);
}

//print out a function postamble
void cgfuncpostamble(symt *sym) 
{
  	cglabel(sym->st_endlabel);
  	fprintf(Outfile, "\taddq\t$%d,%%rsp\n", stackOffset);
  	fputs("\tpopq	%rbp\n" "\tret\n", Outfile);
  	freeall_registers(NOREG);
}

//load integer literal value into register, return number of reg. for x86-64, we don't need to worry about type.
int cgloadint(int value, int type) 
{
  	//get new register
  	int r = alloc_register();

  	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
  	return r;
}

//load value from variable into register, return number of register, if 
//operation is pre- or post-increment/decrement, also perform this action.
int cgloadvar(symt *sym, int op) 
{
  	int r, postreg, offset=1;

  	//get new register
  	r = alloc_register();

  	//if symbol is pointer, use size of type that it points to as any increment or decrement. 
	//if not, it's one.
  	if(ptrtype(sym->type))
    		offset= typesize(value_at(sym->type), sym->ctype);

  	//negate offset for decrements
  	if(op == A_PREDEC || op == A_POSTDEC)
    		offset= -offset;

  	//if we have pre-operation
  	if(op == A_PREINC || op == A_PREDEC) 
	{
    		//load symbol's address
    		if(sym->class == C_LOCAL || sym->class == C_PARAM)
      			fprintf(Outfile, "\tleaq\t%d(%%rbp), %s\n", sym->st_posn, reglist[r]);
    		else
      			fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", sym->name, reglist[r]);

    		//and change value at that address
    		switch(sym->size) 
		{
      			case 1: fprintf(Outfile, "\taddb\t$%d,(%s)\n", offset, reglist[r]); break;
      			case 4: fprintf(Outfile, "\taddl\t$%d,(%s)\n", offset, reglist[r]); break;
      			case 8: fprintf(Outfile, "\taddq\t$%d,(%s)\n", offset, reglist[r]); break;
    		}
  	}

  	//now load output register with value
  	if(sym->class == C_LOCAL || sym->class == C_PARAM) 
	{
    		switch(sym->size)
	       	{
      			case 1: fprintf(Outfile, "\tmovzbq\t%d(%%rbp), %s\n", sym->st_posn, reglist[r]); break;
      			case 4: fprintf(Outfile, "\tmovslq\t%d(%%rbp), %s\n", sym->st_posn, reglist[r]); break;
      			case 8: fprintf(Outfile, "\tmovq\t%d(%%rbp), %s\n", sym->st_posn, reglist[r]);
    		}
  	} 
	else 
	{
    		switch(sym->size) 
		{
      			case 1: fprintf(Outfile, "\tmovzbq\t%s(%%rip), %s\n", sym->name, reglist[r]); break;
      			case 4: fprintf(Outfile, "\tmovslq\t%s(%%rip), %s\n", sym->name, reglist[r]); break;
      			case 8: fprintf(Outfile, "\tmovq\t%s(%%rip), %s\n", sym->name, reglist[r]);
    		}
  	}

  	//if we have post-operation, get new register
  	if(op == A_POSTINC || op == A_POSTDEC) 
	{
    		postreg = alloc_register();

    		//load symbol's address
    		if(sym -> class == C_LOCAL || sym -> class == C_PARAM)
      			fprintf(Outfile, "\tleaq\t%d(%%rbp), %s\n", sym->st_posn, reglist[postreg]);
    		else
      			fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", sym->name, reglist[postreg]);
    		//and change value at that address

    		switch(sym -> size) 
    		{
      			case 1: fprintf(Outfile, "\taddb\t$%d,(%s)\n", offset, reglist[postreg]); break;
      			case 4: fprintf(Outfile, "\taddl\t$%d,(%s)\n", offset, reglist[postreg]); break;
      			case 8: fprintf(Outfile, "\taddq\t$%d,(%s)\n", offset, reglist[postreg]); break;
    		}
    		//and free register
    		free_register(postreg);
  	}

  	//return register with value
  	return r;
}

//given label number of global string, load its address into new register
int cgloadglobstr(int label) 
{
  	//get new register
  	int r = alloc_register();

  	fprintf(Outfile, "\tleaq\tL%d(%%rip), %s\n", label, reglist[r]);
  	
	return r;
}

//add two registers together and return number of register with result
int cgadd(int r1, int r2) 
{
  	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r2);
  	
	return r1;
}

//subtract second register from first and return number of register with result
int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);

  	free_register(r2);

  	return r1;
}

//multiply two registers together and return number of register with result
int cgmul(int r1, int r2) 
{
  	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r2], reglist[r1]);
  	
	free_register(r2);
  	
	return r1;
}

//divide or modulo first register by second and return number of register with result
int cgdivmod(int r1, int r2, int op) 
{
  	fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
  	fprintf(Outfile, "\tcqo\n");
  	fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
  	if(op == A_DIVIDE)
    		fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
  	else
    		fprintf(Outfile, "\tmovq\t%%rdx,%s\n", reglist[r1]);
  	
	free_register(r2);
  	
	return r1;
}

int cgand(int r1, int r2) 
{
  	fprintf(Outfile, "\tandq\t%s, %s\n", reglist[r2], reglist[r1]);
  	
	free_register(r2);
  
	return r1;
}

int cgor(int r1, int r2) 
{
  	fprintf(Outfile, "\torq\t%s, %s\n", reglist[r2], reglist[r1]);
  	
	free_register(r2);
  	
	return r1;
}

int cgxor(int r1, int r2) 
{
  	fprintf(Outfile, "\txorq\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r2);
  	
	return r1;
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

//negate register's value
int cgnegate(int r) 
{
  	fprintf(Outfile, "\tnegq\t%s\n", reglist[r]);
  	
	return r;
}

//invert register's value
int cginvert(int r) 
{
  	fprintf(Outfile, "\tnotq\t%s\n", reglist[r]);
  	return r;
}

//logicall negate a register's value
int cglognot(int r) 
{
  	fprintf(Outfile, "\ttest\t%s, %s\n", reglist[r], reglist[r]);
  	fprintf(Outfile, "\tsete\t%s\n", breglist[r]);
  	fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);
  	
	return r;
}

//load boolean value (only 0 or 1) into the given register
void cgloadboolean(int r, int val) 
{
  	fprintf(Outfile, "\tmovq\t$%d, %s\n", val, reglist[r]);
}


//convert integer value to boolean value, jump if it's IF, WHILE, LOGAND or LOGOR operation
int cgboolean(int r, int op, int label) 
{
  	fprintf(Outfile, "\ttest\t%s, %s\n", reglist[r], reglist[r]);
  	switch(op) 
	{
    		case A_IF:
    		case A_WHILE:
    		case A_LOGAND:
      			fprintf(Outfile, "\tje\tL%d\n", label);
      			break;
    		case A_LOGOR:
      			fprintf(Outfile, "\tjne\tL%d\n", label);
      			break;
    		default:
      			fprintf(Outfile, "\tsetnz\t%s\n", breglist[r]);
      			fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r], reglist[r]);
  	}
  	
	return r;
}

//call function with given symbol id pop off any arguments pushed on stack return register with result
int cgcall(symt *sym, int numargs) 
{
  	int outr;

  	//call function
  	fprintf(Outfile, "\tcall\t%s@PLT\n", sym->name);

  	//remove any arguments pushed on stack
  	if(numargs > 6)
    		fprintf(Outfile, "\taddq\t$%d, %%rsp\n", 8 * (numargs - 6));

  	//unspill all registers
  	unspill_all_regs();

  	//get new register and copy return value into it
  	outr = alloc_register();
  	fprintf(Outfile, "\tmovq\t%%rax, %s\n", reglist[outr]);
  	
	return outr;
}

//given register with argument value, copy this argument into argposn'th
//parameter in preparation for future function call.
//note that argposn is 1, 2, 3, 4, ..., never zero.
void cgcopyarg(int r, int argposn) 
{
	//if this is above sixth argument, simply push 
	//register on the stack, we rely on being called with
  	//successive arguments in correct order for x86-64
  	if(argposn > 6) 
	{
    		fprintf(Outfile, "\tpushq\t%s\n", reglist[r]);
  	} 
	else 
	{
    		//otherwise, copy value into one of six registers used to hold parameter values
    		fprintf(Outfile, "\tmovq\t%s, %s\n", reglist[r], reglist[FIRSTPARAMREG - argposn + 1]);
  	}
  	free_register(r);
}

//shift register left by constant
int cgshlconst(int r, int val) 
{
  	fprintf(Outfile, "\tsalq\t$%d, %s\n", val, reglist[r]);
  	
	return r;
}

//store register's value into variable
int cgstorglob(int r, symt *sym) 
{
  	if(cgprimsize(sym -> type) == 8) 
	{
    		fprintf(Outfile, "\tmovq\t%s, %s(%%rip)\n", reglist[r], sym->name);
  	} 
	else
    		switch(sym->type) 
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

//store register's value into local variable
int cgstorlocal(int r, symt *sym) 
{
  	if(cgprimsize(sym->type) == 8) 
	{
    		fprintf(Outfile, "\tmovq\t%s, %d(%%rbp)\n", reglist[r], sym->st_posn);
  	}
       	else
    		switch(sym->type) 
		{
      			case P_CHAR:
				fprintf(Outfile, "\tmovb\t%s, %d(%%rbp)\n", breglist[r], sym -> st_posn);
				break;
      			case P_INT:
				fprintf(Outfile, "\tmovl\t%s, %d(%%rbp)\n", dreglist[r], sym -> st_posn);
				break;
      			default:
				fatald("Bad type in cgstorlocal:", sym->type);
    		}
  	return r;
}

//generate global symbol but not functions
void cgglobsym(symt *node) 
{
  	int size, type;
  	int initvalue;
  	int i;

  	if(node == NULL)
    		return;
  	
	if(node->stype == S_FUNCTION)
    		return;

  	//get size of variable (or its elements if an array) and type of variable
  	if(node->stype == S_ARRAY) 
	{
    		size = typesize(value_at(node -> type), node -> ctype);
    		type = value_at(node -> type);
  	} 
	else 
	{
    		size = node -> size;
    		type = node -> type;
  	}

  	//generate global identity and label
  	cgdataseg();
  	if(node->class == C_GLOBAL)
    		fprintf(Outfile, "\t.globl\t%s\n", node->name);
  	
	fprintf(Outfile, "%s:\n", node->name);

  	//output space for one or more elements
  	for(i = 0; i < node->nelems; i++) 
	{

    		//get any initial value
    		initvalue = 0;
    		if(node -> initlist != NULL)
      			initvalue = node -> initlist[i];

    		//generate space for this type
    		switch(size) 
		{
      			case 1:
				fprintf(Outfile, "\t.byte\t%d\n", initvalue);
				break;
      			case 4:
				fprintf(Outfile, "\t.long\t%d\n", initvalue);
				break;
      			case 8:
				//generate pointer to string literal, treat zero value
				//as actually zero, not label L0
				if(node->initlist != NULL && type == pointer_to(P_CHAR) && initvalue != 0)
	  				fprintf(Outfile, "\t.quad\tL%d\n", initvalue);
				else
	  				fprintf(Outfile, "\t.quad\t%d\n", initvalue);
				break;
      			default:
				for(i = 0; i < size; i++)
	  				fprintf(Outfile, "\t.byte\t0\n");
    		}
  	}
}

//generate global string and its start label don't output label if append is true...
void cgglobstr(int l, char *strvalue, int append)
{
  	char *cptr;
  	if(!append)
    		cglabel(l);
  	
	for(cptr = strvalue; *cptr; cptr++) 
	{
    		fprintf(Outfile, "\t.byte\t%d\n", *cptr);
  	}
}

void cgglobstrend(void) 
{
  	fprintf(Outfile, "\t.byte\t0\n");
}

//list of comparison instructions, in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *cmplist[] = {"sete", "setne", "setl", "setg", "setle", "setge"};

//compare two registers and set if true.
int cgcompare_and_set(int ASTop, int r1, int r2) 
{
  	//check range of AST operation
  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  	fprintf(Outfile, "\t%s\t%s\n", cmplist[ASTop - A_EQ], breglist[r2]);
  	fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);
  	free_register(r1);
  	
	return r2;
}

//generate label
void cglabel(int l) 
{
  	fprintf(Outfile, "L%d:\n", l);
}

//generate jump to label
void cgjump(int l) 
{
  	fprintf(Outfile, "\tjmp\tL%d\n", l);
}

//list of inverted jump instructions, in AST order:
			    // A_EQ, A_NE, A_LT,  A_GT,  A_LE, A_GE
static char *invcmplist[] = { "jne", "je", "jge", "jle", "jg", "jl" };

//compare two registers and jump if false.
int cgcompare_and_jump(int ASTop, int r1, int r2, int label) 
{
  	//check the range of the AST operation
  	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  	fprintf(Outfile, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], label);
  	freeall_registers(NOREG);
  	
	return NOREG;
}

//widen value in register from old to new type, and return register with this new value
int cgwiden(int r, int oldtype, int newtype) 
{
  	//nothing to do
  	return r;
}

//generate code to return value from function
void cgreturn(int reg, symt *sym) 
{
  	//only return value if we have value to return
  	if(reg != NOREG) 
	{
    		//deal with pointers here as we can't put them in switch statement
    		if(ptrtype(sym->type))
      			fprintf(Outfile, "\tmovq\t%s, %%rax\n", reglist[reg]);
    		else 
		{
      			//generate code depending on function's type
      			switch(sym->type) 
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
	  				fatald("Bad function type in cgreturn:", sym->type);
      			}
    		}
  	}

  	cgjump(sym -> st_endlabel);
}

//generate code to load address of identifier into variable, return new register
int cgaddress(symt *sym) 
{
  	int r = alloc_register();

  	if(sym->class == C_GLOBAL || sym->class == C_STATIC)
    		fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", sym->name, reglist[r]);
  	else
    		fprintf(Outfile, "\tleaq\t%d(%%rbp), %s\n", sym->st_posn, reglist[r]);
  	
	return r;
}

//dereference pointer to get value it pointing at into same register
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
    		case 4:
      			fprintf(Outfile, "\tmovslq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		case 8:
      			fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
      			break;
    		default:
      			fatald("Can't cgderef on type:", type);
  	}
  	return r;
}

//store through dereferenced pointer
int cgstorderef(int r1, int r2, int type) 
{
  	//get size of type
  	int size = cgprimsize(type);

  	switch(size) 
	{
    		case 1:
      			fprintf(Outfile, "\tmovb\t%s, (%s)\n", breglist[r1], reglist[r2]);
      			break;
    		case 4:
      			fprintf(Outfile, "\tmovl\t%s, (%s)\n", dreglist[r1], reglist[r2]);
      			break;
    		case 8:
      			fprintf(Outfile, "\tmovq\t%s, (%s)\n", reglist[r1], reglist[r2]);
      			break;
    		default:
      			fatald("Can't cgstoderef on type:", type);
  	}
  	return r1;
}

//generate switch jump table and code to load registers and call switch() code
void cgswitch(int reg, int casecount, int toplabel, int *caselabel, int *caseval, int defaultlabel) 
{
  	int i, label;

  	//get label for switch table
  	label = genlabel();
  	cglabel(label);

  	//heuristic, if we have no cases, create one case
  	//which points to default case
  	if(casecount == 0) 
	{
    		caseval[0] = 0;
    		caselabel[0] = defaultlabel;
    		casecount = 1;
  	}
  	
	//generate switch jump table.
  	fprintf(Outfile, "\t.quad\t%d\n", casecount);
  	
	for(i = 0; i < casecount; i++)
    		fprintf(Outfile, "\t.quad\t%d, L%d\n", caseval[i], caselabel[i]);
  	
	fprintf(Outfile, "\t.quad\tL%d\n", defaultlabel);

  	//load specific registers
  	cglabel(toplabel);
  	
	fprintf(Outfile, "\tmovq\t%s, %%rax\n", reglist[reg]);
  	fprintf(Outfile, "\tleaq\tL%d(%%rip), %%rdx\n", label);
  	fprintf(Outfile, "\tjmp\t__switch\n");
}

//move value between registers
void cgmove(int r1, int r2) 
{
  	fprintf(Outfile, "\tmovq\t%s, %s\n", reglist[r1], reglist[r2]);
}
