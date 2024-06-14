#include "defs.h"
#include "data.h"
#include "decl.h"

//parsing of statements

//prototypes
static ast *single_statement(void);

// compound_statement:   // empty, i.e. no statement | statement | statement statements ;

//statement:declaration | expression_statement | function_call | if_statement | while_statement | for_statement
// | return_statement ;

// if_statement: if_head | if_head 'else' compound_statement ;

// if_head: 'if' '(' true_false_expression ')' compound_statement  ;

//parse an "if" statement including any
//optional "else" clause and return its AST
static ast *if_statement(void) 
{
  	ast *condAST, *trueAST, *falseAST = NULL;

  	//check for 'if' and '(' first..
  	match(T_IF, "if");
	lparen();

  	//parse expression which is between parentheses and the ')' following..
  	//check that the tree's operation is a comparison.
  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);
  	rparen();

  	//get the AST for the compound statement
  	trueAST = compound_statement();

  	//if we have an 'else', skip it and get the AST for the compound statement
  	if(Token.token == T_ELSE) 
  	{
    		scan(&Token);
    		falseAST = compound_statement();
  	}
  	//build and return the AST for this statement
  	return mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, NULL, 0);
}


//while_statement: 'while' '(' true_false_expression ')' compound_statement  ;

//parse "while" statement and return its AST
static ast *while_statement(void) 
{
  	ast *condAST, *bodyAST;

  	//check for 'while' '('
  	match(T_WHILE, "while");
  	lparen();

  	//parse the following expression and the ')' following, and check for tree's operation is a comparison.
  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);
  	rparen();

  	bodyAST = compound_statement();

  	return mkastnode(A_WHILE, P_NONE, condAST, NULL, bodyAST, NULL, 0);

}

//for_statement: 'for' '(' preop_statement ';'  true_false_expression ';' postop_statement ') 
//compound_statement  ;


//parse "for" statement and return its AST
static ast *for_statement(void) 
{
  	ast *condAST, *bodyAST;
  	ast *preopAST, *postopAST;
  	ast *tree;

  	//ensure we have 'for' '('
  	match(T_FOR, "for");
  	lparen();

  	preopAST = single_statement();
  	semi();

  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);
  	
  	semi();

  	postopAST = single_statement();
  	rparen();

  	bodyAST = compound_statement();


  	//glue the compound statement and the postop tree
  	tree = mkastnode(A_GLUE, P_NONE, bodyAST, NULL, postopAST, NULL, 0);

  	//make a WHILE loop with the condition and this new body
  	tree = mkastnode(A_WHILE, P_NONE, condAST, NULL, tree, NULL, 0);

  	//and glue the preop tree to the A_WHILE tree
  	return mkastnode(A_GLUE, P_NONE, preopAST, NULL, tree, NULL, 0);
}

//return_statement: 'return' '(' expression ')'  ;

//parse a return statement and return its AST
static ast *return_statement(void) 
{
  	ast *tree;

  	//can't return a value if function returns P_VOID
  	if(Functionid -> type == P_VOID)
    		fatal("Can't return from a void function");

  	//ensure we have 'return' '('
  	match(T_RETURN, "return");
  	lparen();

  	//parse the following expression
  	tree = binexpr(0);

  	//ensure this is compatible with the function's type
  	tree = modify_type(tree, Functionid -> type, 0);
  	if(tree == NULL)
    		fatal("Incompatible type to return");

  	//add on the A_RETURN node
  	tree = mkastunary(A_RETURN, P_NONE, tree, NULL, 0);

  	//get the ')'
  	rparen();
  	
	return tree;
}

//parse a single statement and return its AST
static ast *single_statement(void) 
{
  	int type;

  	switch(Token.token) 
  	{
    		case T_CHAR:
    		case T_INT:
    		case T_LONG:
		        type = parse_type();
      			ident();
      			var_declaration(type, C_LOCAL);
			semi();
      			return NULL;		//dont need ast for var declaration..
    		case T_IF:
      			return if_statement();
    		case T_WHILE:
      			return while_statement();
    		case T_FOR:
      			return for_statement();
    		case T_RETURN:
      			return return_statement();
    		default:
      			// For now, see if this is an expression.
      			// This catches assignment statements.
      			return binexpr(0);
  	}
  	return NULL;
}

//parse a compound statement and ret AST
ast *compound_statement(void) 
{
  	ast *left = NULL;
  	ast *tree;

  	lbrace();

  	while(1) 
  	{
    		//parse single statement
    		tree = single_statement();

    		//some statements needs semi to terminate
    		if(tree != NULL && (tree->op == A_ASSIGN || tree->op == A_RETURN || tree->op == A_FUNCCALL))
      			semi();

    		if(tree != NULL) 
    		{
      			if(left == NULL)
				left = tree;
      			else
				left = mkastnode(A_GLUE, P_NONE, left, NULL, tree, NULL, 0);
    		}

    		if(Token.token == T_RBRACE) 
    		{
      			rbrace();
      			return left;
    		}
  	}
}
