#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//parsing of statements

//prototypes
static ast *single_statement(void);

//compound_statement: no statement | statement | statement statements;

// statement:declaration | expression_statement | function_call | if_statement | while_statement | for_statement
// | return_statement;

// if_statement: if_head | if_head 'else' statement;

// if_head: 'if' '(' true_false_expression ')' statement  ;

static ast *if_statement(void) 
{
  	ast *condAST, *trueAST, *falseAST = NULL;

  	match(T_IF, "if");
  	lparen();

  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		condAST = mkastunary(A_TOBOOL, condAST->type, condAST->ctype, condAST, NULL, 0);
  	
  	rparen();

  	trueAST = single_statement();

  	if (Token.token == T_ELSE) 
  	{
    		scan(&Token);
    		falseAST = single_statement();
  	}
  	
  	return (mkastnode(A_IF, P_NONE, NULL, condAST, trueAST, falseAST, NULL, 0));
}


// while_statement: 'while' '(' true_false_expression ')' statement  ;

static ast *while_statement(void) 
{
  	ast *condAST, *bodyAST;

  	match(T_WHILE, "while");
  	lparen();

  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		condAST = mkastunary(A_TOBOOL, condAST->type, condAST->ctype, condAST, NULL, 0);
  	
  	rparen();

  	Looplevel++;
  	bodyAST = single_statement();
  	Looplevel--;

  	return (mkastnode(A_WHILE, P_NONE, NULL, condAST, NULL, bodyAST, NULL, 0));
}

// for_statement: 'for' '(' expression_list ';' true_false_expression ';' expression_list ')' statement 

static ast *for_statement(void) 
{
  	ast *condAST, *bodyAST;
  	ast *preopAST, *postopAST;
  	ast *tree;

  	match(T_FOR, "for");
  	lparen();

  	preopAST = expression_list(T_SEMI);
  	semi();

  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		condAST = mkastunary(A_TOBOOL, condAST->type, condAST->ctype, condAST, NULL, 0);
  	
  	semi();

  	postopAST = expression_list(T_RPAREN);
  	rparen();

  	Looplevel++;
  	bodyAST = single_statement();
  	Looplevel--;

  	tree = mkastnode(A_GLUE, P_NONE, NULL, bodyAST, NULL, postopAST, NULL, 0);

  	tree = mkastnode(A_WHILE, P_NONE, NULL, condAST, NULL, tree, NULL, 0);

  	return mkastnode(A_GLUE, P_NONE, NULL, preopAST, NULL, tree, NULL, 0);
}

// return_statement: 'return' '(' expression ')'  ;

static ast *return_statement(void) 
{
  	ast *tree= NULL;

  	match(T_RETURN, "return");

  	if(Token.token == T_LPAREN) 
  	{
    		if(Functionid->type == P_VOID)
      			fatal("Can't return from a void function");

    		lparen();

    		tree = binexpr(0);

    		tree = modify_type(tree, Functionid->type, Functionid->ctype, 0);
    		if(tree == NULL)
      			fatal("Incompatible type to return");

    		rparen();
  	}

  	tree = mkastunary(A_RETURN, P_NONE, NULL, tree, NULL, 0);

  	semi();
  	
  	return tree;
}

// break_statement: 'break' ;

static ast *break_statement(void) {

  	if(Looplevel == 0 && Switchlevel == 0)
    		fatal("no loop or switch to break out from");
  	
  	scan(&Token);
  	semi();
  	
  	return mkastleaf(A_BREAK, P_NONE, NULL, NULL, 0);
}

// continue_statement: 'continue' ;

static ast *continue_statement(void) 
{

  	if(Looplevel == 0)
    		fatal("no loop to continue to");
  	
  	scan(&Token);
  	semi();
  	return mkastleaf(A_CONTINUE, P_NONE, NULL, NULL, 0);
}
static ast *switch_statement(void) 
{
  	ast *left, *body, *n, *c;
  	ast *casetree = NULL, *casetail;
  	int inloop = 1, casecount = 0;
  	int seendefault = 0;
  	int ASTop, casevalue;

  	scan(&Token);
  	lparen();

  	left = binexpr(0);
  	rparen();
  	lbrace();

  	if(!inttype(left->type))
    		fatal("Switch expression is not of integer type");

  	n = mkastunary(A_SWITCH, P_NONE, NULL, left, NULL, 0);

  	Switchlevel++;
  	while(inloop) 
  	{
    		switch(Token.token) 
    		{
      			case T_RBRACE:
				if(casecount == 0)
	  				fatal("No cases in switch");
				inloop = 0;
				break;
      			case T_CASE:
      			case T_DEFAULT:
				if (seendefault)
	  				fatal("case or default after existing default");

				if (Token.token == T_DEFAULT) 
				{
	  				ASTop = A_DEFAULT;
	  				seendefault = 1;
	  				scan(&Token);
				} 
				else 
				{
	  				ASTop = A_CASE;
	  				scan(&Token);
	  				left = binexpr(0);
	  				if(left->op != A_INTLIT)
	    					fatal("Expecting integer literal for case value");
	  				
	  				casevalue = left->a_intvalue;

	  				for(c = casetree; c != NULL; c = c->right)
	    					if(casevalue == c->a_intvalue)
	      						fatal("Duplicate case value");
				}

				match(T_COLON, ":");
				casecount++;

				if (Token.token == T_CASE)
	  				body = NULL;
				else
	  				body = compound_statement(1);

				if(casetree == NULL) 
				{
	  			casetree = casetail = mkastunary(ASTop, P_NONE, NULL, body, NULL, casevalue);
				} 
				else 
				{
	  				casetail->right = mkastunary(ASTop, P_NONE, NULL, body, NULL, casevalue);
	  				casetail = casetail->right;
				}
				break;
      			default:
				fatals("Unexpected token in switch", Token.tokstr);
    		}
  	}
  	Switchlevel--;

  	n->a_intvalue = casecount;
  	n->right = casetree;
  	rbrace();

  	return n;
}

static ast *single_statement(void) 
{
  	ast *stmt;
   	symt *ctype;

  	switch (Token.token) 
  	{
    		case T_SEMI:
      			semi();
      			break;
    		case T_LBRACE:
      			lbrace();
      			stmt = compound_statement(0);
      			rbrace();
      			return stmt;
    		case T_IDENT:
      			if(findtypedef(Text) == NULL) 
      			{
				stmt = binexpr(0);
				semi();
				
				return stmt;
      			}
    		case T_CHAR:
    		case T_INT:
    		case T_LONG:
    		case T_STRUCT:
    		case T_UNION:
    		case T_ENUM:
    		case T_TYPEDEF:
      			declaration_list(&ctype, C_LOCAL, T_SEMI, T_EOF, &stmt);
      			semi();
      			return (stmt);		
    		case T_IF:
      			return (if_statement());
    		case T_WHILE:
      			return (while_statement());
    		case T_FOR:
      			return (for_statement());
    		case T_RETURN:
      			return (return_statement());
    		case T_BREAK:
      			return (break_statement());
    		case T_CONTINUE:
      			return (continue_statement());
    		case T_SWITCH:
      			return (switch_statement());
    		default:
      			stmt = binexpr(0);
      			semi();
      			return stmt;
  	}
  	return NULL;
}

ast *compound_statement(int inswitch) 
{
  	ast *left = NULL;
  	ast *tree;

  	while (1) 
  	{
    		if(Token.token == T_RBRACE)
      			return left;
    		if(inswitch && (Token.token == T_CASE || Token.token == T_DEFAULT))
      			return left;

    		tree = single_statement();

    		if(tree != NULL) 
    		{
      			if(left == NULL)
				left = tree;
      			else
				left = mkastnode(A_GLUE, P_NONE, NULL, left, NULL, tree, NULL, 0);
    		}
  	}
  	return NULL;
}
