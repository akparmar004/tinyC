#include "defs.h"
#define extern_
#include "data.h"
#undef extern_
#include "decl.h"
#include <errno.h>
#include <unistd.h>

//compiler setup and top-level execution

//given string with '.' and at least 1-character suffix after '.', change suffix to be given character.
//return new string or NULL if original string could not be modified
char *alter_suffix(char *str, char suffix) 
{
  	char *posn;
  	char *newstr;

  	//clone string
  	if((newstr = strdup(str)) == NULL)
    		return NULL;

  	//find the '.'
  	if((posn = strrchr(newstr, '.')) == NULL)
    		return NULL;

  	//ensure there is a suffix
  	posn++;
  	if(*posn == '\0')
    		return NULL;

  	//change suffix and NULL terminate string
  	*posn = suffix;
  	posn++;
  	*posn = '\0';
  	return newstr;
}

//given input filename, compile that file down to assembly code, return new file's name
static char *do_compile(char *filename) 
{
  	char cmd[TEXTLEN];

  	//change input file's suffix to .s
  	Outfilename = alter_suffix(filename, 's');
  	if(Outfilename == NULL) 
  	{
    		fprintf(stderr, "Error: %s has no suffix, try .c on the end\n", filename);
    		exit(1);
  	}

  	//generate pre-processor command
	//for ex. cpp -nostdinc -isystem /tmp/include //(random)sum.c
  	snprintf(cmd, TEXTLEN, "%s %s %s", CPPCMD, INCDIR, filename);
  	
	//open up pre-processor pipe
  	if((Infile = popen(cmd, "r")) == NULL) 
  	{
    		fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
    		exit(1);
  	}
  	Infilename = filename;

  	//create the output file
  	if((Outfile = fopen(Outfilename, "w")) == NULL) 
  	{
    		fprintf(stderr, "Unable to create %s: %s\n", Outfilename, strerror(errno));
    		exit(1);
  	}

  	Line = 1;			//reset the scanner
  	Linestart = 1;
  	Putback = '\n';
  	clear_symtable();		//clear the symbol table
  	if (O_verbose)
    		printf("compiling %s\n", filename);
  	scan(&Token);			//get first token from input
  	Peektoken.token = 0;		//and set there is no lookahead token
  	genpreamble();			//output preamble
  	global_declarations();		//parse global declarations
  	genpostamble();			//output postamble
  	fclose(Outfile);		//close output file

  	//dump the symbol table if requested
  	if(O_dumpsym) 
  	{
    		printf("Symbols for %s\n", filename);
    		dumpsymtables();
    		fprintf(stdout, "\n\n");
  	}

  	freestaticsyms();		//Free any static symbols in the file
  	return Outfilename;
}

//given an input filename, assemble that file down to object code. Return the object filename
char *do_assemble(char *filename) 
{
  	char cmd[TEXTLEN];
  	int err;

  	char *outfilename = alter_suffix(filename, 'o');
  	if(outfilename == NULL) 
  	{
    		fprintf(stderr, "Error: %s has no suffix, try .s on the end\n", filename);
    		exit(1);
  	}
  	// Build the assembly command and run it
  	snprintf(cmd, TEXTLEN, "%s %s %s", ASCMD, outfilename, filename);
  	if(O_verbose)
    		printf("%s\n", cmd);
  	err = system(cmd);
  	
  	if(err != 0) 
  	{
    		fprintf(stderr, "Assembly of %s failed\n", filename);
    		exit(1);
  	}
  	return outfilename;
}

// Given a list of object files and an output filename, link all of the object filenames together.
void do_link(char *outfilename, char **objlist) 
{
  	int cnt, size = TEXTLEN;
  	char cmd[TEXTLEN], *cptr;
  	int err;

  	// Start with the linker command and the output file
  	cptr = cmd;
  	cnt = snprintf(cptr, size, "%s %s ", LDCMD, outfilename);
  	cptr += cnt;
  	size -= cnt;

  	// Now append each object file
  	while (*objlist != NULL) 
  	{
    		cnt = snprintf(cptr, size, "%s ", *objlist);
    		cptr += cnt;
    		size -= cnt;
    		objlist++;
  	}

  	if(O_verbose)
   		 printf("%s\n", cmd);
  	
  	err = system(cmd);
  	if(err != 0) 
  	{
    		fprintf(stderr, "Linking failed\n");
    		exit(1);
  	}
}

//print out a usage if started incorrectly
static void usage(char *prog) 
{
  	fprintf(stderr, "Usage: %s [-vcSTM] [-o outfile] file [file ...]\n", prog);
  	fprintf(stderr, "       -v give verbose output of the compilation stages\n");
  	fprintf(stderr, "       -c generate object files but don't link them\n");
  	fprintf(stderr, "       -S generate assembly files but don't link them\n");
  	fprintf(stderr, "       -T dump the AST trees for each input file\n");
  	fprintf(stderr, "       -M dump the symbol table for each input file\n");
  	fprintf(stderr, "       -o outfile, produce the outfile executable file\n");
  	exit(1);
}

//main program: check arguments and print a usage if we don't have an argument,
//open up the input file and call scanfile() to scan the tokens in it.
enum 
{ 
	MAXOBJ = 100 
};

int main(int argc, char **argv) 
{
  	char *outfilename = AOUT;
  	char *asmfile, *objfile;
  	char *objlist[MAXOBJ];
  	int i, j, objcnt = 0;

  	//initialising our variables
  	O_dumpAST = 0;
  	O_dumpsym = 0;
  	O_keepasm = 0;
  	O_assemble = 0;
  	O_verbose = 0;
  	O_dolink = 1;

  	//scan command-line options
  	for(i = 1; i < argc; i++) 
  	{
    		//no leading '-', stop scanning for options
    		if (*argv[i] != '-')
      			break;

    		//for each option in this argument
    		for(j = 1; (*argv[i] == '-') && argv[i][j]; j++) 
    		{
      			switch(argv[i][j]) 
      			{
				case 'o':
	  				outfilename = argv[++i];	//save & skip to next argument
	  				break;
				case 'T':
	  				O_dumpAST = 1;
	  				break;
				case 'M':
	  				O_dumpsym = 1;
	  				break;
				case 'c':
	  				O_assemble = 1;
	  				O_keepasm = 0;
	 				O_dolink = 0;
	  				break;
				case 'S':
	  				O_keepasm = 1;
	 				O_assemble = 0;
	 				O_dolink = 0;
	  				break;
				case 'v':
	  				O_verbose = 1;
	  				break;
				default:
	  				usage(argv[0]);
      			}
    		}
  	}

  	//ensure we have at lease one input file argument
  	if(i >= argc)
    		usage(argv[0]);

  	//work on each input file in turn
  	while(i < argc) 
  	{
    		asmfile = do_compile(argv[i]);	//compile the source file

    		if (O_dolink || O_assemble) 
    		{
      			objfile = do_assemble(asmfile);	// Assemble it to object forma
      			if(objcnt == (MAXOBJ - 2)) 
      			{
				fprintf(stderr, "Too many object files for the compiler to handle\n");
				exit(1);
      			}
      			objlist[objcnt++] = objfile;	// Add the object file's name
      			objlist[objcnt] = NULL;	// to the list of object files
    		}

    		if (!O_keepasm)		// Remove the assembly file if
      			unlink(asmfile);		// we don't need to keep it
    		i++;
  	}	

 	// Now link all the object files together
  	if (O_dolink) 
  	{
    		do_link(outfilename, objlist);

    		// If we don't need to keep the object files, then remove them
    		if(!O_assemble) 
    		{
      			for(i = 0; objlist[i] != NULL; i++)
				unlink(objlist[i]);
    		}
  	}

  	return 0;
}
