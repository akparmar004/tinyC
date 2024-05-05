#include "defs.h"
#include "data.h"
#include "decl.h"

//code generator for x86-64

//list of available regis.. and their names
static int freereg[4];
static char *reglist[4] = { "%r8", "%r9", "%r10", "%r11" };

//set all registers as available
void freeall_registers(void) 
{
  	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

//allocate a free register. return the number of the register, die if no available registers.
static int alloc_register(void) 
{
  	for (int i = 0; i < 4; i++) 
	{
    		if (freereg[i]) 
		{
      			freereg[i] = 0;
      			return (i);
    		}
  	}
  	fatal("Out of registers");
}

//return a register to the list of available registers. check for if it's not already there.
static void free_register(int reg) 
{
  	if(freereg[reg] != 0)
	{
		fatald("Error trying to free register", reg);
	}
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

//print out the assembly postamble
void cgpostamble() 
{
  	fputs("\tmovl	$0, %eax\n" "\tpopq	%rbp\n" "\tret\n", Outfile);
}

//load an integer literal value into a regs... return the number of the reg..
int cgloadint(int value) 
{
  	// Get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
  	return (r);
}

//load a value from a variable into a register. return the number of the register
int cgloadglob(char *identifier) 
{
  	// Get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", identifier, reglist[r]);
  	return (r);
}

//add two registers together and return the number of the register with the result
int cgadd(int r1, int r2) 
{
  	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return (r2);
}

//subtract the second register from the first and return the number of the register with the result
int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r2);
  	return (r1);
}

// Multiply two registers together and return
// the number of the register with the result
int cgmul(int r1, int r2) {
  fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]);
  free_register(r1);
  return (r2);
}

// Divide the first register by the second and
// return the number of the register with the result
int cgdiv(int r1, int r2) {
  fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
  fprintf(Outfile, "\tcqo\n");
  fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
  fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
  free_register(r2);
  return (r1);
}

// Call printint() with the given register
void cgprintint(int r) {
  fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
  fprintf(Outfile, "\tcall\tprintint\n");
  free_register(r);
}

// Store a register's value into a variable
int cgstorglob(int r, char *identifier) {
  fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], identifier);
  return (r);
}

// Generate a global symbol
void cgglobsym(char *sym) {
  fprintf(Outfile, "\t.comm\t%s,8,8\n", sym);
}
