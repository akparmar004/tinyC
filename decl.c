#include "defs.h"
#include "data.h"
#include "decl.h"

//parsing of declarations

//parse the declaration of a variable
void var_declaration(void) 
{

  	//ensure we have an 'int' token followed by an identifier and a semicolon. 
	//Text now has the identifier's name.
  	//add it as a known identifier
  	match(T_INT, "int");
  	ident();
  	addglob(Text);
  	genglobsym(Text);
  	semi();
}
