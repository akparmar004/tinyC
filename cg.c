#include "defs.h"
#include "data.h"
#include "decl.h"

//code generator for x86-64

//list of available registers and their names
//we need a list of byte registers, too..
static int freereg[4];
static char *reglist[4]  = { "%r8", "%r9", "%r10", "%r11" };
static char *breglist[4] = { "%r8b", "%r9b", "%r10b", "%r11b" };

//set all registers as available or free..
void freeall_registers(void) 
{
  	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

//allocate a free register. return the number of the register. Die if no available registers.
static int alloc_register(void) 
{
  	for(int i = 0; i < 4; i++) 
	{
    		if(freereg[i]) 
		{
      			freereg[i] = 0;
      			return i;
    		}
  	}
  	fatal("Out of registers");
}

//return a register to the list of available registers, check to see if it's not already there.
static void free_register(int reg) 
{
  	if(freereg[reg] != 0)
    		fatald("Error trying to free register", reg);
  	freereg[reg] = 1;
}

//print out the assembly preamble
void cgpreamble() 
{
  	freeall_registers();
  	fputs("\t.text\n"
		".LC0:\n"
		"\t.string\t\"%d\\n\"\n"
		"printint:\n"
		"\tpushq\t%rbp\n"
		"\tmovq\t%rsp, %rbp\n"
		"\tsubq\t$16, %rsp\n"
		"\tmovl\t%edi, -4(%rbp)\n"
		"\tmovl\t-4(%rbp), %eax\n"
		"\tmovl\t%eax, %esi\n"
		"\tleaq	.LC0(%rip), %rdi\n"
		"\tmovl	$0, %eax\n"
		"\tcall	printf@PLT\n"
		"\tnop\n"
		"\tleave\n"
		"\tret\n"
		"\n"
		"\t.globl\tmain\n"
		"\t.type\tmain, @function\n"
		"main:\n" "\tpushq\t%rbp\n" "\tmovq	%rsp, %rbp\n", Outfile);
}

//print the assembly postamble
void cgpostamble() 
{
  	fputs("\tmovl	$0, %eax\n" "\tpopq	%rbp\n" "\tret\n", Outfile);
}

//load an integer literal value into a register, return the number of the regis..
int cgloadint(int value) 
{
  	//get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
  	return r;
}

//load a value from a variable into a regis.., return the number of that register
int cgloadglob(char *identifier) 
{
  	//get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", identifier, reglist[r]);
  	return r;
}

//add two registers and return the number of the register with the result value back..
int cgadd(int r1, int r2) 
{
  	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

//subtract the second register from the first and return the number of the register with the result
int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r2);
  	return r1;
}

//multiply two registers and return the number of that regis.. with the result
int cgmul(int r1, int r2) 
{
  	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return r2;
}

//divide the first register by the second and return the number of the register with the result
int cgdiv(int r1, int r2) 
{
  	fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
  	fprintf(Outfile, "\tcqo\n");
  	fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
  	fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
  	free_register(r2);
  	return r1;
}

//call printint() with the given register
void cgprintint(int r) 
{
  	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
  	fprintf(Outfile, "\tcall\tprintint\n");
  	free_register(r);
}

//store a register's value into a variable
int cgstorglob(int r, char *identifier) 
{
  	fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], identifier);
  	return r;
}

//generate a global symbol
void cgglobsym(char *sym) 
{
  	fprintf(Outfile, "\t.comm\t%s,8,8\n", sym);
}

//list of comparison instructions, in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *cmplist[] = { "sete", "setne", "setl", "setg", "setle", "setge" };

//compare two registers and set if true.
int cgcompare_and_set(int ASTop, int r1, int r2) 
{

  	// Check the range of the AST operation
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

//generate a jump to a label
void cgjump(int l) 
{
  	fprintf(Outfile, "\tjmp\tL%d\n", l);
}

//list of inverted jump instructions, in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *invcmplist[] = { "jne", "je", "jge", "jle", "jg", "jl" };

//compare two registers and jump if false.
int cgcompare_and_jump(int ASTop, int r1, int r2, int label) 
{
  	//check the range of the AST operation
	if(ASTop < A_EQ || ASTop > A_GE)
    		fatal("Bad ASTop in cgcompare_and_set()");

  	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  	fprintf(Outfile, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], label);
  	freeall_registers();
  	return NOREG;
}
