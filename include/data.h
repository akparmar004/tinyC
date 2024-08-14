#ifndef extern_
#define extern_ extern
#endif

//global variables

extern_ int Line;		     	//current line number
extern_ int Linestart;		     	//true if at start of a line
extern_ int Putback;		     	//character put back by scanner
extern_ symt *Functionid; 		//symbol ptr of the current function
extern_ FILE *Infile;		     	//input and output files
extern_ FILE *Outfile;
extern_ char *Infilename;		//name of file we are parsing
extern_ char *Outfilename;		//name of file we opened as Outfile
extern_ struct token Token;		//last token scanned
extern_ struct token Peektoken;		//look-ahead token
extern_ char Text[TEXTLEN + 1];		//last identifier scanned
extern_ int Looplevel;			//depth of nested loops
extern_ int Switchlevel;		//depth of nested switches
extern char *Tstring[];			//list of token strings

//symbol table lists
extern_ symt *Globhead, *Globtail;	  //global variables and functions
extern_ symt *Loclhead, *Locltail;	  //local variables
extern_ symt *Parmhead, *Parmtail;	  //local parameters
extern_ symt *Membhead, *Membtail;	  //temp list of struct/union members
extern_ symt *Structhead, *Structtail; 	  //list of struct types
extern_ symt *Unionhead, *Uniontail;      //list of union types
extern_ symt *Enumhead,  *Enumtail;       //list of enum types and values
extern_ symt *Typehead,  *Typetail;    	  //list of typedefs

//command-line flags
extern_ int O_dumpAST;		//if true, dump the AST trees
extern_ int O_dumpsym;		//if true, dump the symbol table
extern_ int O_keepasm;		//if true, keep any assembly files
extern_ int O_assemble;		//if true, assemble the assembly files
extern_ int O_dolink;		//if true, link the object files
extern_ int O_verbose;		//if true, print info on compilation stages
