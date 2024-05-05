#include "defs.h"
#include "data.h"
#include "decl.h"

static int freereg[4];
static char *reglist[4] = {"%r8", "%r9", "%r10", "%r11"};

//set all reg. as available 
void freeall_registers(void)
{
	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

//allocate a free register. and return the number of that reg.. //die if no reg.. available.
static int alloc_register(void)
{
	for(int i = 0;i < 4;i++)
	{
		if(freereg[i])
		{
			freereg[i] = 0;
			return i;
		}
	}

	fprintf(stderr, "Out of registers!\n");
	exit(1);
}

static void free_register(int reg)
{
  	if (freereg[reg] != 0) 
  	{
    		fprintf(stderr, "Error trying to free register %d\n", reg);
    		exit(1);
  	}
  	freereg[reg]= 1;
}

//print out the assembly preamble..
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
  	fputs("\tmovl $0, %eax\n" "\tpopq %rbp\n" "\tret\n",Outfile);
}

//this func. will load an int lit value into reg..
int cgload(int value)
{
	//get a new reg..
	int r = alloc_register();

	//print the code to initialise it..	
	fprintf(Outfile,"\tmovq\t$%d, %s\n",value, reglist[r]);
	
	return r;
}

//adds two register together.. and returns the number of the reg. with the result..
int cgadd(int r1, int r2)
{
	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return(r2);
}

int cgmul(int r1, int r2)
{
	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r1);
  	return(r2);
}

//subtraction is not commutative.. so be carefull about order of reg..
int cgsub(int r1, int r2)
{
	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r2);
  	return(r1);
}

//same as subtr.. division is also not commutative
int cgdiv(int r1, int r2)
{
	fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
  	fprintf(Outfile, "\tcqo\n");
  	fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
  	fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
  	free_register(r2);
  	return(r1);
}

//call printint() with the given register
void cgprintint(int r) 
{
  	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
  	fprintf(Outfile, "\tcall\tprintint\n");
  	free_register(r);
}
	
