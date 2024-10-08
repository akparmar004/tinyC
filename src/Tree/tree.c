#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//AST tree functions

ast *mkastnode(int op, int type, symt *ctype, ast *left, ast *mid, ast *right, symt *sym, int intvalue)
{
        ast *n;

        n = (ast*)malloc(sizeof(ast));
        if (n == NULL)
        fatal("Unable to malloc in mkastnode()");

  	n->op = op;
  	n->type = type;
  	n->ctype = ctype;
  	n->left = left;
  	n->mid = mid;
  	n->right = right;
  	n->sym = sym;
 	n->a_intvalue = intvalue;
  	
  	return n;
}

ast *mkastleaf(int op, int type, symt *ctype, symt *sym, int intvalue)
{
  	return mkastnode(op, type, ctype, NULL, NULL, NULL, sym, intvalue);
}

ast *mkastunary(int op, int type, symt *ctype, ast *left, symt *sym, int intvalue)
{
  	return mkastnode(op, type, ctype, left, NULL, NULL, sym, intvalue);
}

static int dumpid = 1;
static int gendumplabel(void)
{
  	return (dumpid++);
}

void dumpAST(ast *n, int label, int level)
{
  	int Lfalse, Lstart, Lend;
  	int i;

  	switch(n->op)
  	{
  		case A_IF:
    			Lfalse = gendumplabel();
    			for (i = 0; i < level; i++)
      				fprintf(stdout, " ");
    			fprintf(stdout, "A_IF");
    			if (n->right)
    			{
      				Lend = gendumplabel();
      				fprintf(stdout, ", end L%d", Lend);
    			}
    			fprintf(stdout, "\n");
    			dumpAST(n->left, Lfalse, level + 2);
    			dumpAST(n->mid, NOLABEL, level + 2);
    			if (n->right)
      				dumpAST(n->right, NOLABEL, level + 2);
      					
    			return;
  		case A_WHILE:
    			Lstart = gendumplabel();
    			for (i = 0; i < level; i++)
      				fprintf(stdout, " ");
    			fprintf(stdout, "A_WHILE, start L%d\n", Lstart);
    			Lend = gendumplabel();
   		 	dumpAST(n->left, Lend, level + 2);
    			dumpAST(n->right, NOLABEL, level + 2);
    			return;
 	}
  	if (n->op == A_GLUE)
    		level = -2;

  	if(n->left)
    		dumpAST(n->left, NOLABEL, level + 2);
  	if(n->right)
    		dumpAST(n->right, NOLABEL, level + 2);

  	for(i = 0; i < level; i++)
    		fprintf(stdout, " ");
  	switch(n->op)
  	{
  		case A_GLUE:
    			fprintf(stdout, "\n\n");
    			return;
  		case A_FUNCTION:
    			fprintf(stdout, "A_FUNCTION %s\n", n->sym->name);
    			return;
  		case A_ADD:
    			fprintf(stdout, "A_ADD\n");
    			return;
  		case A_SUBTRACT:
    			fprintf(stdout, "A_SUBTRACT\n");
    			return;
  		case A_MULTIPLY:
    			fprintf(stdout, "A_MULTIPLY\n");
    			return;
  		case A_DIVIDE:
    			fprintf(stdout, "A_DIVIDE\n");
    			return;
  		case A_EQ:
    			fprintf(stdout, "A_EQ\n");
    			return;
  		case A_NE:
   			fprintf(stdout, "A_NE\n");
    			return;
  		case A_LT:
    			fprintf(stdout, "A_LE\n");
    			return;
  		case A_GT:
    			fprintf(stdout, "A_GT\n");
    			return;
  		case A_LE:
    			fprintf(stdout, "A_LE\n");
    			return;
  		case A_GE:
    			fprintf(stdout, "A_GE\n");
    			return;
  		case A_INTLIT:
    			fprintf(stdout, "A_INTLIT %d\n", n->a_intvalue);
    			return;
  		case A_STRLIT:
    			fprintf(stdout, "A_STRLIT rval label L%d\n", n->a_intvalue);
    			return;
  		case A_IDENT:
    			if (n->rvalue)
      				fprintf(stdout, "A_IDENT rval %s\n", n->sym->name);
    			else
      				fprintf(stdout, "A_IDENT %s\n", n->sym->name);
    			return;
  		case A_ASSIGN:
    			fprintf(stdout, "A_ASSIGN\n");
    			return;
  		case A_WIDEN:
    			fprintf(stdout, "A_WIDEN\n");
    			return;
  		case A_RETURN:
    			fprintf(stdout, "A_RETURN\n");
    			return;
  		case A_FUNCCALL:
    			fprintf(stdout, "A_FUNCCALL %s\n", n->sym->name);
    			return;
  		case A_ADDR:
    			fprintf(stdout, "A_ADDR %s\n", n->sym->name);
    			return;
  		case A_DEREF:
    			if(n->rvalue)
      				fprintf(stdout, "A_DEREF rval\n");
    			else
      				fprintf(stdout, "A_DEREF\n");
    			return;
  		case A_SCALE:
    			fprintf(stdout, "A_SCALE %d\n", n->a_size);
    			return;
  		case A_PREINC:
    			fprintf(stdout, "A_PREINC %s\n", n->sym->name);
    			return;
  		case A_PREDEC:
    			fprintf(stdout, "A_PREDEC %s\n", n->sym->name);
    			return;
  		case A_POSTINC:
    			fprintf(stdout, "A_POSTINC\n");
    			return;
  		case A_POSTDEC:
    			fprintf(stdout, "A_POSTDEC\n");
    			return;
  		case A_NEGATE:
    			fprintf(stdout, "A_NEGATE\n");
    			return;
  		case A_BREAK:
    			fprintf(stdout, "A_BREAK\n");
    			return;
  		case A_CONTINUE:
    			fprintf(stdout, "A_CONTINUE\n");
    			return;
  		case A_CASE:
    			fprintf(stdout, "A_CASE %d\n", n->a_intvalue);
    			return;
  		case A_DEFAULT:
    			fprintf(stdout, "A_DEFAULT\n");
    			return;
  		case A_SWITCH:
    			fprintf(stdout, "A_SWITCH\n");
    			return;
  		case A_CAST:
    			fprintf(stdout, "A_CAST %d\n", n->type);
    			return;
  		case A_ASPLUS:
    			fprintf(stdout, "A_ASPLUS\n");
    			return;
  		case A_ASMINUS:
    			fprintf(stdout, "A_ASMINUS\n");
    			return;
  		case A_ASSTAR:
    			fprintf(stdout, "A_ASSTAR\n");
    			return;
  		case A_ASSLASH:
    			fprintf(stdout, "A_ASSLASH\n");
    			return;
  		case A_TOBOOL:
    			fprintf(stdout, "A_TOBOOL\n");
    			return;
  		case A_LOGOR:
    			fprintf(stdout, "A_LOGOR\n");
    			return;
  		case A_LOGAND:
    			fprintf(stdout, "A_LOGAND\n");
    			return;
  		default:
    			fatald("Unknown dumpAST operator", n->op);
  	}
}
