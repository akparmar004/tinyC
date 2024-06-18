#ifndef extern_
#define extern_ extern
#endif

//global variables

extern_ int Line;		//current line number..
extern_ int Putback;		//character put back by scanner, used to putback scanned char to its place..
extern_ symt *Functionid;	//current functon's symbol id
extern_ int Globs;		//position of next free global symbol slot

extern_ FILE *Infile;		//input file pointer to scan every single char..
extern_ FILE *Outfile;		//outfile file pointer to write assembly code in out file..
extern_ char *Outfilename;		// Name of file we opened as Outfile

extern_ struct token Token;	//token scanner
extern_ char Text[TEXTLEN + 1];		//last identifier scanned

//symbol table list
extern_ struct symtable *Globhead, *Globtail;	// Global variables and functions
extern_ struct symtable *Loclhead, *Locltail;	// Local variables
extern_ struct symtable *Parmhead, *Parmtail;	// Local parameters
extern_ struct symtable *Comphead, *Comptail;   // Composite types

extern_ int O_dumpAST;
extern_ int O_keepasm;		// If true, keep any assembly files
extern_ int O_assemble;		// If true, assemble the assembly files
extern_ int O_dolink;		// If true, link the object files
extern_ int O_verbose;		// If true, print info on compilation stages
