#include "defs.h"
#define extern_
#include "data.h"
#undef extern_
#include "decl.h"
#include <errno.h>

//compiler setup and top-level execution

//initialise global variables
static void init() 
{
  	Line = 1;
  	Putback = '\n';
  	Globs = 0;
  	O_dumpAST = 0;
}

//print out a usage if started incorrectly
static void usage(char *prog) 
{
  	fprintf(stderr, "Usage: %s [-T] infile\n", prog);
  	exit(1);
}

//main program: check arguments and print a usage
//if we don't have an argument. Open up the input
//file and call scanfile() to scan the tokens in it.
int main(int argc, char *argv[]) 
{
  	int i;

  	//initialise the globals
  	init();

  	//scan for command-line options
  	for(i=1; i<argc; i++) 
	{
    		if(*argv[i] != '-') 
			break;
    		
		for(int j=1; argv[i][j]; j++) 
		{
      			switch(argv[i][j]) 
			{
				case 'T': O_dumpAST =1;
					break;
				default : usage(argv[0]);
      			}
    		}
  	}

  	//ensure we have an input file argument
  	if(i >= argc)
    		usage(argv[0]);

  	
	//open up the input file
  	if((Infile = fopen(argv[i], "r")) == NULL) 
	{
    		fprintf(stderr, "Unable to open %s: %s\n", argv[i], strerror(errno));
    		exit(1);
  	}
  	
	//create the output file
  	if((Outfile = fopen("out.s", "w")) == NULL) 
	{
    		fprintf(stderr, "Unable to create out.s: %s\n", strerror(errno));
    		exit(1);
  	}
  	
	//for now, ensure that void printint() is defined
  	addglob("printint", P_CHAR, S_FUNCTION, 0, 0);

  	scan(&Token);			//get the first token from the input
  	genpreamble();			//output the preamble
  	
	global_declarations();		// Parse the global declarations
  	
	genpostamble();			// Output the postamble
  	fclose(Outfile);		// Close the output file and exit
  	
	return 0;
}
