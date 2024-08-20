#include "../../include/defs.h"
#define extern_
#include "../../include/data.h"
#undef extern_
#include "../../include/decl.h"
#include <errno.h>
#include <unistd.h>

//compiler main file

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

static char *do_compile(char *filename) 
{
  	char cmd[TEXTLEN];

  	Outfilename = alter_suffix(filename, 'q');
  	if(Outfilename == NULL) 
  	{
    		fprintf(stderr, "Error: %s has no suffix, try .c on the end\n", filename);
    		exit(1);
  	}

  	snprintf(cmd, TEXTLEN, "%s %s %s", CPPCMD, INCDIR, filename);
  	
  	if((Infile = popen(cmd, "r")) == NULL) 
  	{
    		fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
    		exit(1);
  	}
  	Infilename = filename;

  	if((Outfile = fopen(Outfilename, "w")) == NULL) 
  	{
    		fprintf(stderr, "Unable to create %s: %s\n", Outfilename, strerror(errno));
    		exit(1);
  	}

  	Line = 1;			
  	Linestart = 1;
  	Putback = '\n';
  	clear_symtable();		
  	if (O_verbose)
    		printf("compiling %s\n", filename);
  	scan(&Token);			
  	Peektoken.token = 0;		
  	genpreamble(filename);			
  	global_declarations();		
  	genpostamble();			
  	fclose(Outfile);		

  	if(O_dumpsym) 
  	{
    		printf("Symbols for %s\n", filename);
    		dumpsymtables();
    		fprintf(stdout, "\n\n");
  	}

  	freestaticsyms();		//Free any static symbols in file
  	return Outfilename;
}

char *do_qbe(char *filename) 
{
  	char cmd[TEXTLEN];
  	int err;

  	char *outfilename = alter_suffix(filename, 's');
  	if(outfilename == NULL) 
  	{
    		fprintf(stderr, "Error: %s has no suffix, try .qbe on the end\n", filename);
    		exit(1);
  	}
  	
  	snprintf(cmd, TEXTLEN, "%s %s %s", QBECMD, outfilename, filename);
  	if(O_verbose)
    		printf("%s\n", cmd);
  	
  	err = system(cmd);
  	if(err != 0) 
  	{
    		fprintf(stderr, "QBE translation of %s failed\n", filename);
    		exit(1);
  	}
  	
  	return outfilename;
}

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

void do_link(char *outfilename, char **objlist) 
{
  	int cnt, size = TEXTLEN;
  	char cmd[TEXTLEN], *cptr;
  	int err;

  	cptr = cmd;
  	cnt = snprintf(cptr, size, "%s %s ", LDCMD, outfilename);
  	cptr += cnt;
  	size -= cnt;

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

enum { MAXOBJ = 100 };

int main(int argc, char **argv) 
{
  	char *outfilename = AOUT;
  	char *qbefile, *asmfile, *objfile;
  	char *objlist[MAXOBJ];
  	int i, j, objcnt = 0;

  	O_dumpAST = 0;
  	O_dumpsym = 0;
  	O_keepasm = 0;
  	O_assemble = 0;
  	O_verbose = 0;
  	O_dolink = 1;

  	//scan command-line options
  	for(i = 1; i < argc; i++) 
  	{
    		if (*argv[i] != '-')
      			break;

    		for(j = 1; (*argv[i] == '-') && argv[i][j]; j++) 
    		{
      			switch(argv[i][j]) 
      			{
				case 'o':
	  				outfilename = argv[++i];	
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

  	if(i >= argc)
    		usage(argv[0]);

  	while(i < argc) 
  	{
    		qbefile = do_compile(argv[i]);	
    		asmfile = do_qbe(qbefile);

    		if(O_dolink || O_assemble) 
    		{
      			objfile = do_assemble(asmfile);	
      			if(objcnt == (MAXOBJ - 2)) 
      			{
				fprintf(stderr, "Too many object files for the compiler to handle\n");
				exit(1);
      			}
      			objlist[objcnt++] = objfile;	
      			objlist[objcnt] = NULL;	
    		}

    		if (!O_keepasm)		
    		{
    			unlink(qbefile);
      			unlink(asmfile);		
      		}
    		i++;
  	}	

  	if (O_dolink) 
  	{
    		do_link(outfilename, objlist);
    		
		if(!O_assemble) 
    		{
      			for(i = 0; objlist[i] != NULL; i++)
				unlink(objlist[i]);
    		}
  	}

  	return 0;
}
