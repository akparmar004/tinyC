#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

static symt *composite_declaration(int type);
static int typedef_declaration(symt **ctype);
static int type_of_typedef(char *name, symt **ctype);
static void enum_declaration(void);

int parse_type(symt **ctype, int *class) 
{
  	int type = 0, exstatic = 1;

  	while(exstatic) 
	{
    		switch(Token.token) 
		{
			case T_EXTERN:
				if (*class == C_STATIC)
	  				fatal("Illegal to have extern and static at the same time");
				
				*class = C_EXTERN;
				scan(&Token);
				break;
      			case T_STATIC:
				if(*class == C_LOCAL)
	  				fatal("Compiler doesn't support static local declarations");
				
				if(*class == C_EXTERN)
	  				fatal("Illegal to have extern and static at the same time");
				
				*class = C_STATIC;
				scan(&Token);
				break;
      			default:
				exstatic = 0;
    		}
  	}

  	switch(Token.token) 
	{
    		case T_VOID:
      			type = P_VOID;
      			scan(&Token);
      			break;
    		case T_CHAR:
      			type = P_CHAR;
      			scan(&Token);
      			break;
    		case T_INT:
      			type = P_INT;
      			scan(&Token);
      			break;
    		case T_LONG:
      			type = P_LONG;
      			scan(&Token);
      			break;

    		case T_STRUCT:
      			type = P_STRUCT;
      			*ctype = composite_declaration(P_STRUCT);
      			if(Token.token == T_SEMI)
				type = -1;
      			break;
    		case T_UNION:
      			type = P_UNION;
      			*ctype = composite_declaration(P_UNION);
      			if(Token.token == T_SEMI)
				type = -1;
      			break;
    		case T_ENUM:
      			type = P_INT;		
      			enum_declaration();
      			if(Token.token == T_SEMI)
				type = -1;
      			break;
    		case T_TYPEDEF:
      			type = typedef_declaration(ctype);
      			if(Token.token == T_SEMI)
				type = -1;
      			break;
    		case T_IDENT:
      			type = type_of_typedef(Text, ctype);
      			break;
    		default:
      			fatals("Illegal type, token", Token.tokstr);
  	}
  	return type;
}

int parse_stars(int type) 
{
 	while(1)
 	{
    		if(Token.token != T_STAR)
      			break;
    		
    		type = pointer_to(type);
    		scan(&Token);
  	}
  	return type;
}

int parse_cast(symt **ctype) 
{
  	int type, class = 0;

  	type = parse_stars(parse_type(ctype, &class));

  	if(type == P_STRUCT || type == P_UNION || type == P_VOID)
    		fatal("Cannot cast to a struct, union or void type");
  	
  	return type;
}

int parse_literal(int type) 
{
  	ast *tree;

  	tree = optimise(binexpr(0));

  	if(tree->op == A_CAST) 
  	{
    		tree->left->type = tree->type;
    		tree = tree->left;
  	}
  	if(tree->op != A_INTLIT && tree->op != A_STRLIT)
    		fatal("Cannot initialise globals with a general expression");

  	if(type == pointer_to(P_CHAR)) 
  	{
    		if(tree -> op == A_STRLIT)
      			return(tree->a_intvalue);
    		if(tree->op == A_INTLIT && tree->a_intvalue == 0)
      			return 0;
  	}
  	if(inttype(type) && typesize(type, NULL) >= typesize(tree->type, NULL))
    		return (tree->a_intvalue);

  	fatal("Type mismatch: literal vs. variable");
  	
  	return 0;			
}

int is_new_symbol(symt *sym, int class, int type, symt *ctype) 
{

  	if(sym==NULL) return(1);

  	if((sym->class== C_GLOBAL && class== C_EXTERN) || (sym->class== C_EXTERN && class== C_GLOBAL)) 
  	{

      		if(type != sym->type)
        		fatals("Type mismatch between global/extern", sym->name);

      		if(type >= P_STRUCT && ctype != sym->ctype)
        		fatals("Type mismatch between global/extern", sym->name);

      		sym -> class = C_GLOBAL;
      		
      		return 0;
  	}

  	fatals("Duplicate global variable declaration", sym->name);
  	return -1;	
}

static symt *scalar_declaration(char *varname, int type, symt *ctype, int class, ast **tree) 
{
  	symt *sym = NULL;
  	ast *varnode, *exprnode;
  	*tree = NULL;

  	switch(class) 
  	{
    		case C_STATIC:
    		case C_EXTERN:
    		case C_GLOBAL:
	        
      			sym = findglob(varname);
      			if(is_new_symbol(sym, class, type, ctype))
        			sym = addglob(varname, type, ctype, S_VARIABLE, class, 1, 0);
      		
      			break;
    		case C_LOCAL:
      			sym = addlocl(varname, type, ctype, S_VARIABLE, 1);
      			break;
    		case C_PARAM:
      			sym = addparm(varname, type, ctype, S_VARIABLE);
      			break;
    		case C_MEMBER:
      			sym = addmemb(varname, type, ctype, S_VARIABLE, 1);
      			break;
  	}

  	if(Token.token == T_ASSIGN) 
  	{
    		if(class != C_GLOBAL && class != C_LOCAL && class != C_STATIC)
      			fatals("Variable can not be initialised", varname);
    		scan(&Token);

    		if(class == C_GLOBAL || class == C_STATIC) 
    		{
      			sym->initlist = (int *) malloc(sizeof(int));
      			sym->initlist[0] = parse_literal(type);
    		}
    		if(class == C_LOCAL) 
    		{
      			varnode = mkastleaf(A_IDENT, sym->type, sym->ctype, sym, 0);

      			exprnode = binexpr(0);
      			exprnode->rvalue = 1;

      			exprnode = modify_type(exprnode, varnode->type, varnode->ctype, 0);
      			if(exprnode == NULL)
				fatal("Incompatible expression in assignment");

      		*tree = mkastnode(A_ASSIGN, exprnode->type, exprnode->ctype, exprnode, NULL, varnode, NULL, 0);
    		}
  	}
  	if(class == C_GLOBAL || class == C_STATIC)
    		genglobsym(sym);

  	return sym;
}

static symt *array_declaration(char *varname, int type, symt *ctype, int class) 
{
  	symt *sym = NULL;		
	int nelems = -1;		
  	int maxelems;			
  	int *initlist;		
  	int i = 0, j;

  	scan(&Token);

  	if(Token.token != T_RBRACKET) 
  	{
    		nelems = parse_literal(P_INT);
    		if(nelems <= 0)
      			fatald("Array size is illegal", nelems);
  	}
  	match(T_RBRACKET, "]");

  	switch(class) 
  	{
    		case C_STATIC:
    		case C_EXTERN:
    		case C_GLOBAL:
      		sym = findglob(varname);
      		if(is_new_symbol(sym, class, pointer_to(type), ctype))
        		sym = addglob(varname, pointer_to(type), ctype, S_ARRAY, class, 0, 0);
      		break;

    		case C_LOCAL:
      			sym = addlocl(varname, pointer_to(type), ctype, S_ARRAY, 0);
			sym -> st_hasaddr = 1;
      		break;
    		default:
      			fatal("Declaration of array parameters is not implemented");
  	}

  	if(Token.token == T_ASSIGN) 
  	{
    		if(class != C_GLOBAL && class != C_STATIC)
      			fatals("Variable can not be initialised", varname);
    		
    		scan(&Token);

    		match(T_LBRACE, "{");

#define TABLE_INCREMENT 10

    		if(nelems != -1)
      			maxelems = nelems;
    		else
      			maxelems = TABLE_INCREMENT;
    		
    		initlist = (int *) malloc(maxelems * sizeof(int));

    		while(1)
    		{

      			if(nelems != -1 && i == maxelems)
				fatal("Too many values in initialisation list");

      			initlist[i++] = parse_literal(type);

      			if(nelems == -1 && i == maxelems) 
      			{
				maxelems += TABLE_INCREMENT;
				initlist = (int *) realloc(initlist, maxelems * sizeof(int));
      			}
      			if(Token.token == T_RBRACE) 
      			{
				scan(&Token);
				break;
      			}
      			comma();
    		}

    		for(j = i; j < sym->nelems; j++)
      			initlist[j] = 0;
    		
    		if(i > nelems)
      			nelems = i;
    		
    		sym -> initlist = initlist;
  	}

  	if(class != C_EXTERN && nelems<=0)
    		fatals("Array must have non-zero elements", sym->name);

  	sym->nelems = nelems;
  	sym->size = sym->nelems * typesize(type, ctype);
  	
  	if(class == C_GLOBAL || class == C_STATIC)
    		genglobsym(sym);
  	return sym;
}

static int param_declaration_list(symt *oldfuncsym, symt *newfuncsym) 
{
  	int type, paramcnt = 0;
  	symt *ctype;
  	symt *protoptr = NULL;
  	ast *unused;

  	if(oldfuncsym != NULL)
    		protoptr = oldfuncsym->member;

  	while(Token.token != T_RPAREN) 
  	{

    		if(Token.token == T_VOID) 
    		{
      			scan(&Peektoken);
      			if(Peektoken.token == T_RPAREN) 
      			{
				// Move the Peektoken into the Token
				paramcnt = 0;
				scan(&Token);
				break;
      			}
    		}
    		type = declaration_list(&ctype, C_PARAM, T_COMMA, T_RPAREN, &unused);
    		if(type == -1)
      			fatal("Bad type in parameter list");

    		if(protoptr != NULL) 
    		{
      			if(type != protoptr->type)
				fatald("Type doesn't match prototype for parameter", paramcnt + 1);
      			
      			protoptr = protoptr->next;
    		}
    		paramcnt++;

    		if(Token.token == T_RPAREN)
      			break;
    		
    		comma();
  	}

  	if(oldfuncsym != NULL && paramcnt != oldfuncsym->nelems)
    		fatals("Parameter count mismatch for function", oldfuncsym->name);

  	return paramcnt;
}

// function_declaration: type identifier '(' parameter_list ')'; | 
// 			 type identifier '(' parameter_list ')' compound_statement;

static symt *function_declaration(char *funcname, int type, symt *ctype, int class) 
{
  	ast *tree, *finalstmt;
  	symt *oldfuncsym, *newfuncsym = NULL;
  	int endlabel = 0, paramcnt;
	int linenum = Line;
  	if((oldfuncsym = findsymbol(funcname)) != NULL)
    		if(oldfuncsym->stype != S_FUNCTION)
      			oldfuncsym = NULL;

  	if(oldfuncsym == NULL) 
  	{
    		endlabel = genlabel();
    		
		newfuncsym = addglob(funcname, type, NULL, S_FUNCTION, class, 0, endlabel);
  	}
  	
  	lparen();
  	paramcnt = param_declaration_list(oldfuncsym, newfuncsym);
  	rparen();

  	if(newfuncsym) 
  	{
    		newfuncsym->nelems = paramcnt;
    		newfuncsym->member = Parmhead;
    		oldfuncsym = newfuncsym;
  	}
  	Parmhead = Parmtail = NULL;

  	if(Token.token == T_SEMI)
    		return oldfuncsym;

  	Functionid = oldfuncsym;

  	Looplevel = 0;
  	Switchlevel = 0;
  	lbrace();
  	tree = compound_statement(0);
  		rbrace();

  	if(type != P_VOID) 
  	{

    		if(tree == NULL)
      			fatal("No statements in function with non-void type");

    		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
    		if(finalstmt == NULL || finalstmt->op != A_RETURN)
      			fatal("No return for function with non-void type");
  	}
  	tree = mkastunary(A_FUNCTION, type, ctype, tree, oldfuncsym, endlabel);

	tree -> linenum = linenum;
  	tree = optimise(tree);

  	if(O_dumpAST) 
  	{
    		dumpAST(tree, NOLABEL, 0);
    		fprintf(stdout, "\n\n");
  	}
  	genAST(tree, NOLABEL, NOLABEL, NOLABEL, 0);

  	freeloclsyms();
  	return oldfuncsym;
}

static symt *composite_declaration(int type) 
{
  	symt *ctype = NULL;
  	symt *m;
  	ast *unused;
  	int offset;
  	int t;

  	scan(&Token);

  	if(Token.token == T_IDENT) 
  	{
    		if(type == P_STRUCT)
      			ctype = findstruct(Text);
    		else
      			ctype = findunion(Text);
    		
    		scan(&Token);
  	}
  	if(Token.token != T_LBRACE) 
  	{
    		if(ctype == NULL)
      			fatals("unknown struct/union type", Text);
    	
    		return ctype;
  	}
  	if(ctype)
    		fatals("previously defined struct/union", Text);

  	if(type == P_STRUCT)
    		ctype = addstruct(Text);
  	else
    		ctype = addunion(Text);
  	
  	scan(&Token);

  	while(1) 
  	{
    		t = declaration_list(&m, C_MEMBER, T_SEMI, T_RBRACE, &unused);
    		if(t == -1)
      			fatal("Bad type in member list");
    		
    		if(Token.token == T_SEMI)
      			scan(&Token);
    		
    		if(Token.token == T_RBRACE)
      			break;
  	}

  	rbrace();
  	if(Membhead == NULL)
    		fatals("No members in struct", ctype->name);
  	
  	ctype -> member = Membhead;
  	Membhead = Membtail = NULL;

  	m = ctype->member;
  	m->st_posn = 0;
  	offset = typesize(m->type, m->ctype);

  	for(m = m->next; m != NULL; m = m->next) 
  	{
    		if(type == P_STRUCT)
      			m->st_posn = genalign(m->type, offset, 1);
    		else
      			m->st_posn = 0;
    		
		offset += typesize(m->type, m->ctype);
  	}

  	ctype->size = offset;
  	return ctype;
}

static void enum_declaration(void) 
{
  	symt *etype = NULL;
	char *name = NULL;
  	int intval = 0;

  	scan(&Token);

  	if(Token.token == T_IDENT) 
  	{
    		etype = findenumtype(Text);
    		name = strdup(Text);	// As it gets tromped soon
    		scan(&Token);
  	}
  	if(Token.token != T_LBRACE) 
  	{
    		if(etype == NULL)
      			fatals("undeclared enum type:", name);
    		
    		return;
  	}
  	scan(&Token);

  	if(etype != NULL)
    		fatals("enum type redeclared:", etype->name);
  	else
    		etype = addenum(name, C_ENUMTYPE, 0);

  	while(1) 
  	{
    		ident();
    		name = strdup(Text);

   		etype = findenumval(name);
    		if(etype != NULL)
      			fatals("enum value redeclared:", Text);

    		if(Token.token == T_ASSIGN) 
    		{
      			scan(&Token);
      			if(Token.token != T_INTLIT)
				fatal("Expected int literal after '='");
      			
      			intval = Token.intvalue;
      			scan(&Token);
    		}
    		etype = addenum(name, C_ENUMVAL, intval++);

    		if(Token.token == T_RBRACE)
      			break;
    		comma();
  	}
  	scan(&Token);			
}

static int typedef_declaration(symt **ctype) 
{
  	int type, class = 0;

  	scan(&Token);

  	type = parse_type(ctype, &class);
  	if(class != 0)
    		fatal("Can't have extern in a typedef declaration");
  	
	if(findtypedef(Text) != NULL)
    		fatals("redefinition of typedef", Text);

  	type = parse_stars(type);

  	addtypedef(Text, type, *ctype);
  	scan(&Token);
  	
  	return type;
}

static int type_of_typedef(char *name, symt **ctype) 
{
   	symt *t;

  	t = findtypedef(name);
  	if(t == NULL)
    		fatals("unknown type", name);
  	
  	scan(&Token);
  	*ctype = t->ctype;
  	
  	return t->type;
}

static symt *symbol_declaration(int type, symt *ctype, int class, ast **tree) 
{
  	symt *sym = NULL;
  	char *varname = strdup(Text);

  	ident();

  	if(Token.token == T_LPAREN) 
  	{
    		return (function_declaration(varname, type, ctype, class));
  	}
  	switch(class) 
  	{
    		case C_EXTERN:
    		case C_STATIC:
    		case C_GLOBAL:
    		case C_LOCAL:
    		case C_PARAM:
      			if(findlocl(varname) != NULL)
				fatals("Duplicate local variable declaration", varname);
    		
    		case C_MEMBER:
      			if(findmember(varname) != NULL)
				fatals("Duplicate struct/union member declaration", varname);
  	}

  	if(Token.token == T_LBRACKET) 
  	{
    		sym = array_declaration(varname, type, ctype, class);
    		*tree= NULL;	
  	} 
  	else
    		sym = scalar_declaration(varname, type, ctype, class, tree);
  	
  	return sym;
}

int declaration_list(symt **ctype, int class, int et1, int et2, ast **gluetree) 
{	
  	int inittype, type;
  	symt *sym;
  	ast *tree = NULL;
  	*gluetree = NULL;

  	if((inittype = parse_type(ctype, &class)) == -1)
    		return inittype;

  	while(1) 
  	{
    		type = parse_stars(inittype);

    		sym = symbol_declaration(type, *ctype, class, &tree);

    		if(sym->stype == S_FUNCTION) 
    		{
      			if(class != C_GLOBAL && class != C_STATIC)
				fatal("Function definition not at global level");
      			
      			return type;
    		}
    		if(*gluetree == NULL)
      			*gluetree = tree;
    		else
     			*gluetree = mkastnode(A_GLUE, P_NONE, NULL, *gluetree, NULL, tree, NULL, 0);

    		if(Token.token == et1 || Token.token == et2)
      			return type;

    		comma();
  	}
  	
  	return 0;	
}

void global_declarations(void) 
{
  	symt *ctype = NULL;
  	ast *unused;

  	while(Token.token != T_EOF) 
  	{
    		declaration_list(&ctype, C_GLOBAL, T_SEMI, T_EOF, &unused);
    		
    		if(Token.token == T_SEMI)
      			scan(&Token);
  	}
}
