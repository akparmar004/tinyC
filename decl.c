#include "defs.h"
#include "data.h"
#include "decl.h"

//parsing of declarations

// declaration: 'int' identifier ';'  ;
//parse declaration of a variable
void var_declaration(void) 
{
  	//check for int keyword before identifier and a semicolon. 
	//in Text we have now identifier's name and add it as a known identifier
  	match(T_INT, "int");
  	ident();
  	addglob(Text);
  	genglobsym(Text);
	semi();
}

ast* function_declaration(void)
{
	ast *tree;
	int nameslot;

	match(T_VOID, "void");
	ident();
	nameslot = addglob(Text);
	lparen();
	rparen();

	tree = compound_statement();

	return mkastunary(A_FUNCTION, tree, nameslot);
}
