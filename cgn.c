#include "defs.h"
#include "data.h"
#include "decl.h"

//code generator for x86-64

//list of available registers and their names
static int freereg[4];
static char *reglist[4]= { "r8", "r9", "r10", "r11" };

//set all registers as available
void freeall_registers(void)
{
	freereg[0]= freereg[1]= freereg[2]= freereg[3]= 1;
}

//allocate a free register. return the number of the register. Die if no available registers.
static int alloc_register(void)
{
  	for (int i=0; i<4; i++) 
  	{
  	  	if(freereg[i]) 
  	  	{
		      freereg[i]= 0;
		      return(i);
    		}
  	}
  	fprintf(stderr, "Out of registers!\n");
  	exit(1);
}

//return a regs.. to the list of available regs..
//check to see if it's not already there.
static void free_register(int reg)
{
  	if (freereg[reg] != 0) 
  	{
  		fprintf(stderr, "Error trying to free register %d\n", reg);
  		exit(1);
  	}
  	freereg[reg]= 1;
}

//print out the assembly preamble
void cgpreamble()
{
  	freeall_registers();
  fputs("\tglobal\tmain\n"
	"\textern\tprintf\n"
	"\tsection\t.text\n"
	"LC0:\tdb\t\"%d\",10,0\n"
	"printint:\n"
	"\tpush\trbp\n"
	"\tmov\trbp, rsp\n"
	"\tsub\trsp, 16\n"
	"\tmov\t[rbp-4], edi\n"
	"\tmov\teax, [rbp-4]\n"
	"\tmov\tesi, eax\n"
	"\tlea	rdi, [rel LC0]\n"
	"\tmov	eax, 0\n"
	"\tcall	printf\n"
	"\tnop\n"
	"\tleave\n"
	"\tret\n"
	"\n"
	"main:\n" "\tpush\trbp\n" "\tmov rbp, rsp\n", Outfile);
}

//print out the assembly postamble
void cgpostamble()
{
  	fputs("\tmov eax, 0\n" "\tpop rbp\n" "\tret\n", Outfile);
}

//load an integer literal value into a regs.. return the number of the regis..
int cgload(int value) 
{

	//get a new register
	int r= alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmov\t%s, %d\n", reglist[r], value);
  	return(r);
}

//add two regs.. together and return the number of the regis.. with the result
int cgadd(int r1, int r2) 
{
  	fprintf(Outfile, "\tadd\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r1);
  	return(r2);
}

//subtract the second register from the first and return the number of the register with the result
int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsub\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r2);
  	return(r1);
}

//multiply two registers together and return the number of the register with the result
int cgmul(int r1, int r2) 
{
  	fprintf(Outfile, "\timul\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r1);
  	return(r2);
}

//divide the first register by the second and return the number of the register with the result
int cgdiv(int r1, int r2) 
{
  	fprintf(Outfile, "\tmov\trax, %s\n", reglist[r1]);
  	fprintf(Outfile, "\tcqo\n");
  	fprintf(Outfile, "\tidiv\t%s\n", reglist[r2]);
  	fprintf(Outfile, "\tmov\t%s, rax\n", reglist[r1]);
  	free_register(r2);
  	return(r1);
}

//call printint() with the given register
void cgprintint(int r) 
{
  	fprintf(Outfile, "\tmov\trdi, %s\n", reglist[r]);
  	fprintf(Outfile, "\tcall\tprintint\n");
  	free_register(r);
}
