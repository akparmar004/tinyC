#include "defs.h"
#include "data.h"
#include "decl.h"

//code generator for x86-64

//list of available registers and their names
static int freereg[4];
static char *reglist[4] = { "r8", "r9", "r10", "r11" };
static char *breglist[4] = { "r8b", "r9b", "r10b", "r11b" };

//set all registers as available
void freeall_registers(void) 
{
  	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}

//allocate a free register. return the number of the register.die if no available registers.
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

//return a register to the list of available registers. check to see if it's not already there.
static void free_register(int reg) 
{
  	if (freereg[reg] != 0)
    		fatald("Error trying to free register", reg);

  	freereg[reg] = 1;
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
	"main:\n" "\tpush\trbp\n" "\tmov	rbp, rsp\n", Outfile);
}

//print out the assembly postamble
void cgpostamble() 
{
  	fputs("\tmov	eax, 0\n" "\tpop	rbp\n" "\tret\n", Outfile);
}	

//load an integer literal value into a regs.. return the number of the regs..
int cgloadint(int value) 
{
  	//get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmov\t%s, %d\n", reglist[r], value);
  	return (r);
}

//load a value from a variable into a regs.. return the number of the regs..
int cgloadglob(char *identifier) 
{
  	//get a new register
  	int r = alloc_register();

  	//print out the code to initialise it
  	fprintf(Outfile, "\tmov\t%s, [%s]\n", reglist[r], identifier);
  	return (r);
}

//add two registers together and return the number of the register with the result
int cgadd(int r1, int r2) 
{
  	fprintf(Outfile, "\tadd\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r1);
  	return (r2);
}

//subtract the second register from the first and return the number of the register with the result
int cgsub(int r1, int r2) 
{
  	fprintf(Outfile, "\tsub\t%s, %s\n", reglist[r1], reglist[r2]);
  	free_register(r2);
  	return (r1);
}

//multiply two registers together and return the number of the register with the result
int cgmul(int r1, int r2) 
{
  	fprintf(Outfile, "\timul\t%s, %s\n", reglist[r2], reglist[r1]);
  	free_register(r1);
  	return (r2);
}

//divide the first regs.. by the second and return the number of the regs.. with the result
int cgdiv(int r1, int r2) 
{
  	fprintf(Outfile, "\tmov\trax, %s\n", reglist[r1]);
  	fprintf(Outfile, "\tcqo\n");
  	fprintf(Outfile, "\tidiv\t%s\n", reglist[r2]);
  	fprintf(Outfile, "\tmov\t%s, rax\n", reglist[r1]);
  	free_register(r2);
  	return (r1);
}

//call printint() with the given register
void cgprintint(int r) 
{
  	fprintf(Outfile, "\tmov\trdi, %s\n", reglist[r]);
  	fprintf(Outfile, "\tcall\tprintint\n");
  	free_register(r);
}

//store a register's value into a variable
int cgstorglob(int r, char *identifier) 
{
  	fprintf(Outfile, "\tmov\t[%s], %s\n", identifier, reglist[r]);
  	return (r);
}

//generate a global symbol
void cgglobsym(char *sym) 
{
  	fprintf(Outfile, "\tcommon\t%s 8:8\n", sym);
}


// List of comparison instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *cmplist[] =
  { "sete", "setne", "setl", "setg", "setle", "setge" };

// Compare two registers and set if true.
int cgcompare_and_set(int ASTop, int r1, int r2) {

  // Check the range of the AST operation
  if (ASTop < A_EQ || ASTop > A_GE)
    fatal("Bad ASTop in cgcompare_and_set()");

  fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  fprintf(Outfile, "\t%s\t%s\n", cmplist[ASTop - A_EQ], breglist[r2]);
  fprintf(Outfile, "\tmovzb\t%s, %s\n", reglist[r2], breglist[r2]);
  free_register(r1);
  return (r2);
}

// Generate a label
void cglabel(int l) {
  fprintf(Outfile, "L%d:\n", l);
}

// Generate a jump to a label
void cgjump(int l) {
  fprintf(Outfile, "\tjmp\tL%d\n", l);
}

// List of inverted jump instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char *invcmplist[] = { "jne", "je", "jge", "jle", "jg", "jl" };

// Compare two registers and jump if false.
int cgcompare_and_jump(int ASTop, int r1, int r2, int label) {

  // Check the range of the AST operation
  if (ASTop < A_EQ || ASTop > A_GE)
    fatal("Bad ASTop in cgcompare_and_set()");

  fprintf(Outfile, "\tcmp\t%s, %s\n", reglist[r1], reglist[r2]);
  fprintf(Outfile, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], label);
  freeall_registers();
  return (NOREG);
}
