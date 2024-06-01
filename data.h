#ifndef extern_
#define extern_ extern
#endif

//global variables

extern_ int Line;		//current line number..
extern_ int Putback;		//character put back by scanner, used to putback scanned char to its place..
extern_ int Functionid;		//current functon's symbol id
extern_ int Globs;		//position of next free global symbol slot

extern_ FILE *Infile;		//input file pointer to scan every single char..
extern_ FILE *Outfile;		//outfile file pointer to write assembly code in out file..

extern_ struct token Token;	//token scanner
extern_ symt Gsym[NSYMBOLS];	//global symbol table
extern_ char Text[TEXTLEN + 1];		//last identifier scanned

extern_ int O_dumpAST;
