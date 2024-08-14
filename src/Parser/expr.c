#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//parsing of expressions

// expression_list: <null> | expression | expression ',' expression_list

ast *expression_list(int endtoken) 
{
  	ast *tree = NULL;
  	ast *child = NULL;
  	int exprcount = 0;

 	while(Token.token != endtoken) 
  	{

    		child = binexpr(0);
    		exprcount++;

    		tree = mkastnode(A_GLUE, P_NONE, NULL, tree, NULL, child, NULL, exprcount);

    		if(Token.token == endtoken)
      			break;

    		match(T_COMMA, ",");
  	}

  	return tree;
}

static ast *funccall(void) 
{
  	ast *tree;
  	symt *funcptr;

  	if((funcptr = findsymbol(Text)) == NULL || funcptr->stype != S_FUNCTION) 
  	{
    		fatals("Undeclared function", Text);
  	}
  	lparen();

  	tree = expression_list(T_RPAREN);

  	tree = mkastunary(A_FUNCCALL, funcptr->type, funcptr->ctype, tree, funcptr, 0);

  	rparen();
  	
  	return tree;
}

static ast *array_access(ast *left) 
{
  	ast *right;

  	if(!ptrtype(left->type))
    		fatal("Not an array or pointer");

  	scan(&Token);

  	right = binexpr(0);

  	match(T_RBRACKET, "]");

  	if(!inttype(right->type))
    		fatal("Array index is not of integer type");

  	left->rvalue = 1;

  	right = modify_type(right, left->type, left->ctype, A_ADD);

  	left = mkastnode(A_ADD, left->type, left->ctype, left, NULL, right, NULL, 0);
  	left = mkastunary(A_DEREF, value_at(left->type), left->ctype, left, NULL, 0);
  	
  	return left;
}

static ast *member_access(ast *left, int withpointer) {
  	ast *right;
  	symt *typeptr;
  	symt *m;

  	if(withpointer && left->type != pointer_to(P_STRUCT) && left->type != pointer_to(P_UNION))
    		fatal("Expression is not a pointer to a struct/union");

  	if(!withpointer) 
  	{
    		if(left->type == P_STRUCT || left->type == P_UNION)
      			left->op = A_ADDR;
    		else
      			fatal("Expression is not a struct/union");
  	}

  	typeptr = left->ctype;

  	scan(&Token);
  	ident();

  	for(m = typeptr->member; m != NULL; m = m->next)
    		if(!strcmp(m->name, Text))
      			break;
  	if(m == NULL)
    		fatals("No member found in struct/union: ", Text);

  	left->rvalue = 1;

  	right = mkastleaf(A_INTLIT, P_INT, NULL, NULL, m->st_posn);

  	left = mkastnode(A_ADD, pointer_to(m->type), m->ctype, left, NULL, right, NULL, 0);
  	left = mkastunary(A_DEREF, m->type, m->ctype, left, NULL, 0);
  	
  	return left;
}

static ast *paren_expression(int ptp) 
{
  	ast *n;
  	int type = 0;
  	symt *ctype = NULL;

  	scan(&Token);

  	switch(Token.token) 
  	{
  		case T_IDENT:
    			if(findtypedef(Text) == NULL) 
    			{
      				n = binexpr(0);	// ptp is zero as expression inside ( )
      				break;
    			}
  		case T_VOID:
  		case T_CHAR:
  		case T_INT:
  		case T_LONG:
  		case T_STRUCT:
  		case T_UNION:
  		case T_ENUM:
    			type = parse_cast(&ctype);

    			rparen();

  		default:
  			n = binexpr(ptp);	
  	}

  	if(type == 0)
    		rparen();
  	else
    		n = mkastunary(A_CAST, type, ctype, n, NULL, 0);
  	return n;
}

static ast *primary(int ptp) 
{
  	ast *n;
  	symt *enumptr;
  	symt *varptr;
  	int id;
  	int type = 0;
  	int size, class;
  	symt *ctype;

  	switch(Token.token) 
  	{
  		case T_STATIC:
  		case T_EXTERN:
    			fatal("Compiler doesn't support static or extern local declarations");
  		case T_SIZEOF:
    			scan(&Token);
    			if(Token.token != T_LPAREN)
      				fatal("Left parenthesis expected after sizeof");
    			scan(&Token);

    			type = parse_stars(parse_type(&ctype, &class));

   	 		size = typesize(type, ctype);
    			rparen();

    			return (mkastleaf(A_INTLIT, P_INT, NULL, NULL, size));

  		case T_INTLIT:
    			if(Token.intvalue >= 0 && Token.intvalue < 256)
      				n = mkastleaf(A_INTLIT, P_CHAR, NULL, NULL, Token.intvalue);
    			else
      				n = mkastleaf(A_INTLIT, P_INT, NULL, NULL, Token.intvalue);
    			break;

  		case T_STRLIT:
    			id = genglobstr(Text, 0);

    			while(1) 
    			{
      				scan(&Peektoken);
      				if(Peektoken.token != T_STRLIT) 
      					break;
      				genglobstr(Text, 1);
      				scan(&Token);	// To skip it properly
    			}

    			genglobstrend();
    			n = mkastleaf(A_STRLIT, pointer_to(P_CHAR), NULL, NULL, id);
    			break;

  		case T_IDENT:
    			if((enumptr = findenumval(Text)) != NULL) 
    			{
      				n = mkastleaf(A_INTLIT, P_INT, NULL, NULL, enumptr->st_posn);
      				break;
    			}
    			if((varptr = findsymbol(Text)) == NULL)
      				fatals("Unknown variable or function", Text);
    			
    			switch(varptr->stype) 
    			{
    				case S_VARIABLE:
      					n = mkastleaf(A_IDENT, varptr->type, varptr->ctype, varptr, 0);
      					break;
    				case S_ARRAY:
      					n = mkastleaf(A_ADDR, varptr->type, varptr->ctype, varptr, 0);
      					n->rvalue = 1;
      					break;
    				case S_FUNCTION:
      					scan(&Token);
      					if(Token.token != T_LPAREN)
						fatals("Function name used without parentheses", Text);
      					
      					return funccall();
    				default:
      					fatals("Identifier not a scalar or array variable", Text);
    			}	
    			break;

  		case T_LPAREN:
    			return (paren_expression(ptp));

  		default:
    			fatals("Expecting a primary expression, got token", Token.tokstr);
  	}

  	scan(&Token);
  	return n;
}

static ast *postfix(int ptp) 
{
  	ast *n;

  	n = primary(ptp);

  	while(1) 
  	{
    		switch(Token.token) 
    		{
    			case T_LBRACKET:
      				n = array_access(n);
      				break;

    			case T_DOT:
      				n = member_access(n, 0);
      				break;

    			case T_ARROW:
      				n = member_access(n, 1);
      				break;

    			case T_INC:
      				if(n->rvalue == 1)
					fatal("Cannot ++ on rvalue");
      				
      				scan(&Token);

      				if(n->op == A_POSTINC || n->op == A_POSTDEC)
					fatal("Cannot ++ and/or -- more than once");

      				n -> op = A_POSTINC;
      				break;
      				
    			case T_DEC:
      				if(n->rvalue == 1)
					fatal("Cannot -- on rvalue");
      				scan(&Token);

      				if(n->op == A_POSTINC || n->op == A_POSTDEC)
					fatal("Cannot ++ and/or -- more than once");

      				n -> op = A_POSTDEC;
      				break;

    			default:
      				return n;
    		}
  	}

  	return NULL;		
}


static int binastop(int tokentype) 
{
  	if(tokentype > T_EOF && tokentype <= T_MOD)
    		return tokentype;
  	fatals("Syntax error, token", Tstring[tokentype]);
  	
  	return 0;
}

static int rightassoc(int tokentype) 
{
  	if(tokentype >= T_ASSIGN && tokentype <= T_ASSLASH)
    		return 1;
  	
  	return 0;
}

static int OpPrec[] = 
{
  	0, 10, 10,	
  	10, 10,			
  	10, 10,		
  	15,				
  	20, 30,			
  	40, 50, 60,			
  	70, 70,			
  	80, 80, 80, 80,		
  	90, 90,			
  	100, 100,			
  	110, 110, 110
};

static int op_precedence(int tokentype) 
{
  	int prec;
  	if(tokentype > T_MOD)
    		fatals("Token with no precedence in op_precedence:", Tstring[tokentype]);
  	
  	prec = OpPrec[tokentype];
  	if(prec == 0)
    		fatals("Syntax error, token", Tstring[tokentype]);
  	
  	return prec;
}

static ast *prefix(int ptp) 
{
  	ast *tree;
  	switch(Token.token) 
  	{
  		case T_AMPER:
    			scan(&Token);
    			tree = prefix(ptp);

    			if (tree->op != A_IDENT)
      				fatal("& operator must be followed by an identifier");

    			if (tree->sym->stype == S_ARRAY)
      				fatal("& operator cannot be performed on an array");

    			tree->op = A_ADDR;
    			tree->type = pointer_to(tree->type);
    			break;
  		case T_STAR:
    			scan(&Token);
    			tree = prefix(ptp);

    			if (!ptrtype(tree->type))
      				fatal("* operator must be followed by an expression of pointer type");

    			tree = mkastunary(A_DEREF, value_at(tree->type), tree->ctype, tree, NULL, 0);
    			break;
  		case T_MINUS:
    			scan(&Token);
    			tree = prefix(ptp);

    			tree->rvalue = 1;
    			if(tree->type == P_CHAR)
      				tree->type = P_INT;
    			tree = mkastunary(A_NEGATE, tree->type, tree->ctype, tree, NULL, 0);
    			break;
  		case T_INVERT:
    			scan(&Token);
    			tree = prefix(ptp);

    			tree->rvalue = 1;
    			tree = mkastunary(A_INVERT, tree->type, tree->ctype, tree, NULL, 0);
    			break;
  		case T_LOGNOT:
    			scan(&Token);
    			tree = prefix(ptp);

    			tree->rvalue = 1;
    			tree = mkastunary(A_LOGNOT, tree->type, tree->ctype, tree, NULL, 0);
    			break;
  		case T_INC:
    			scan(&Token);
    			tree = prefix(ptp);

    			if(tree->op != A_IDENT)
      				fatal("++ operator must be followed by an identifier");

    			tree = mkastunary(A_PREINC, tree->type, tree->ctype, tree, NULL, 0);
    			break;
  		case T_DEC:
    			scan(&Token);
    			tree = prefix(ptp);

    			if (tree->op != A_IDENT)
      				fatal("-- operator must be followed by an identifier");

    			tree = mkastunary(A_PREDEC, tree->type, tree->ctype, tree, NULL, 0);
    			break;
  		default:
    			tree = postfix(ptp);
  	}
  	return tree;
}

ast *binexpr(int ptp) 
{
  	ast *left, *right;
  	ast *ltemp, *rtemp;
  	int ASTop;
  	int tokentype;

  	left = prefix(ptp);

  	tokentype = Token.token;
  	if(tokentype == T_SEMI || tokentype == T_RPAREN || tokentype == T_RBRACKET || tokentype == T_COMMA ||
      	tokentype == T_COLON || tokentype == T_RBRACE) 
     	{
    		left->rvalue = 1;
    		return left;
  	}
  	while((op_precedence(tokentype) > ptp) || (rightassoc(tokentype) && op_precedence(tokentype) == ptp)) 
  	{
    		scan(&Token);

    		right = binexpr(OpPrec[tokentype]);

    		ASTop = binastop(tokentype);

    		switch(ASTop) 
    		{
    			case A_TERNARY:
      				match(T_COLON, ":");
      				ltemp = binexpr(0);

      			return (mkastnode(A_TERNARY, right->type, right->ctype, left, right, ltemp,NULL, 0));

    			case A_ASSIGN:
      				right->rvalue = 1;

      				right = modify_type(right, left->type, left->ctype, 0);
     				if (right == NULL)
					fatal("Incompatible expression in assignment");

     				ltemp = left;
      				left = right;
      				right = ltemp;
      				break;

    			default:
      				left->rvalue = 1;
      				right->rvalue = 1;

      				ltemp = modify_type(left, right->type, right->ctype, ASTop);
     				rtemp = modify_type(right, left->type, left->ctype, ASTop);
      				
      				if(ltemp == NULL && rtemp == NULL)
					fatal("Incompatible types in binary expression");
      				
      				if(ltemp != NULL)
					left = ltemp;
      				if(rtemp != NULL)
					right = rtemp;
    		}

   		left = mkastnode(binastop(tokentype), left->type, left->ctype, left, NULL, right, NULL, 0);

    		switch(binastop(tokentype)) 
    		{
    			case A_LOGOR:
    			case A_LOGAND:
    			case A_EQ:
    			case A_NE:
    			case A_LT:
    			case A_GT:
    			case A_LE:
    			case A_GE:
      				left->type = P_INT;
    		}

    		tokentype = Token.token;
    		if(tokentype == T_SEMI || tokentype == T_RPAREN || 
    		   tokentype == T_RBRACKET || tokentype == T_COMMA ||
		   tokentype == T_COLON || tokentype == T_RBRACE) 
		{
      			left->rvalue = 1;
      			return left;
    		}
  	}

  	left->rvalue = 1;
  	
  	return left;
}
