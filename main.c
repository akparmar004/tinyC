#include "defs.h"
#define extern_
#include "data.h"
#undef extern_
#include "decl.h"
#include <errno.h>

//initialise global variables
static void init() 
{
	Line = 1;
	Putback = '\n';
}

//print out a usage if started incorrectly
static void usage(char *prog) 
{
  	fprintf(stderr, "Usage: %s infile\n", prog);
  	exit(1);
}

//list of printable tokens
char *tokstr[] = { "+", "-", "*", "/", "intlit" };

//loop scanning in all the tokens in the input file.
//print out details of each token found.
static void scanfile() 
{
  	struct token T;

  	while(scan(&T)) 
  	{
    		printf("Token %s", tokstr[T.token]);
    		if(T.token == T_INTLIT)
	  	      printf(", value %d", T.intvalue);	
    		printf("\n");
  	}
}

//main program: check arguments and print a usage if we don't have an argument. Open up the input file and call scanfile() to scan the tokens in it.
void main(int argc, char *argv[]) 
{
	ast *n;
	if(argc != 2)
    		usage(argv[0]);

	init();

	if ((Infile = fopen(argv[1], "r")) == NULL) 
	{
    		fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
    		exit(1);
  	}
	
	scan(&Token);
	n = binexpr();
	printf("%d\n",interpretAST(n));
        exit(0);
}
