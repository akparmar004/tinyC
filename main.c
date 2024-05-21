#include "defs.h"
#define extern_
#include "data.h"
#undef extern_
#include "decl.h"
#include <errno.h>

//compiler setup and top-level execution

//initialise global variables line and Putback..
static void init() 
{
  	Line = 1;
  	Putback = '\n';
}

//print out a usage if started incorrectly, i.e. didn't get input file.., or entered wrong file name..
static void usage(char *prog) 
{
  	fprintf(stderr, "Usage: %s infile\n", prog);
  	exit(1);
}

//main program - check for arguments and print a usage if we don't have an argument. 
//open up the input file start scanning with first initial scan() func. call..
void main(int argc, char *argv[]) {
  	ast *tree;

  	if(argc != 2)
    		usage(argv[0]);

  	init(); //initialize global vari..

  	//open input file
  	if((Infile = fopen(argv[1], "r")) == NULL) 
	{
    		fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
    		exit(1);
  	}
  	
	//create output file
  	if((Outfile = fopen("out.s", "w")) == NULL) 
	{
    		fprintf(stderr, "Unable to create out.s: %s\n", strerror(errno));
    		exit(1);
  	}

  	scan(&Token);			//get the first token from the input file
				
  	genpreamble();			//output the preamble, initial code for assembly..
	while(1)
	{
		tree = function_declaration();
		genAST(tree, NOREG, 0);
		if(Token.token == T_EOF)
			break;	
	}
		
  	fclose(Outfile);		//close the output file and exit
  	exit(0);
}
