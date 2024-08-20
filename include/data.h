#ifndef extern_
#define extern_ extern
#endif

//global variables

extern_ int Line;		     	
extern_ int Linestart;		     	
extern_ int Putback;		     	
extern_ symt *Functionid; 	
extern_ FILE *Infile;		     	
extern_ FILE *Outfile;
extern_ char *Infilename;		
extern_ char *Outfilename;		
extern_ struct token Token;		
extern_ struct token Peektoken;		
extern_ char Text[TEXTLEN + 1];		
extern_ int Looplevel;			
extern_ int Switchlevel;		
extern char *Tstring[];	

//symbol table lists
extern_ symt *Globhead, *Globtail;	 
extern_ symt *Loclhead, *Locltail;	  
extern_ symt *Parmhead, *Parmtail;	  
extern_ symt *Membhead, *Membtail;	  
extern_ symt *Structhead, *Structtail; 
extern_ symt *Unionhead, *Uniontail;  
extern_ symt *Enumhead,  *Enumtail;    
extern_ symt *Typehead,  *Typetail;    

//command-line flags
extern_ int O_dumpAST;		
extern_ int O_dumpsym;		
extern_ int O_keepasm;		
extern_ int O_assemble;		
extern_ int O_dolink;		
extern_ int O_verbose;
