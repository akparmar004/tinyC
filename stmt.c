#include "defs.h"
#include "data.h"
#include "decl.h"

void statements(void) 
{
  	struct ASTnode *tree;
  	int reg;

	while (1) 
	{
    		//match a 'print' as the first token
    		match(T_PRINT, "print");

    		//parse the following expression and
    		//generate the assembly code
    		tree = binexpr(0);
    		reg = genAST(tree);
    		genprintint(reg);
    		genfreeregs();

    		//match the following semicolon and stop if we are at EOF
    		semi();
    		if(Token.token == T_EOF)
      			return;
  	}
}
