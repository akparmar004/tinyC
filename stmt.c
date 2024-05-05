#include "defs.h"
#include "data.h"
#include "decl.h"

void print_statement(void) 
{
  	struct ASTnode *tree;
  	int reg;
   	
	//match a 'print' as the first token
    	match(T_PRINT, "print");

    	//generate the assembly code
	tree = binexpr(0);
	reg = genAST(tree, -1);
	genprintint(reg);
	genfreeregs();

	//match semi colon
	semi();
}

void assignment_statement(void) 
{
  	struct ASTnode *left, *right, *tree;
  	int id;

  	//ensure we have an identifier
  	ident();

  	//check it's been defined then make a leaf node for it
  	if ((id = findglob(Text)) == -1) 
  	{
    		fatals("Undeclared variable", Text);
  	}
        right = mkastleaf(A_LVIDENT, id);

  	//ensure we have an equals sign
  	match(T_EQUALS, "=");

  	//parse the following expression
  	left = binexpr(0);

  	//make an assignment AST tree
  	tree = mkastnode(A_ASSIGN, left, right, 0);

  	//generate the assembly code for the assignment
  	genAST(tree, -1);
  	genfreeregs();

  	//match the following semicolon
  	semi();
}


//parse one or more statements
void statements(void) 
{

  	while (1) 
  	{
    		switch (Token.token) 
    		{
    			case T_PRINT:
				print_statement();
      				break;
    			case T_INT:
      				var_declaration();
      				break;
    			case T_IDENT:
      				assignment_statement();
      				break;
    			case T_EOF:
      				return;
    			default:
      				fatald("Syntax error, token", Token.token);
    		}
  	}
}
