#include "defs.h" 
#include "data.h"
#include "decl.h"     

//parsing of statements
static ast *single_statement(void);

static ast *print_statement(void) 
{
 	ast *tree;
  	int reg;

  	//first match print token..
  	match(T_PRINT, "print");

  	//parse the attached expression
  	tree = binexpr(0);

  	//make an print AST tree
  	tree = mkastunary(A_PRINT, tree, 0);

  	//return the tree.. (AST)
	return tree;
}

//assignment_statement: identifier '=' expression ';' ;

static ast *assignment_statement(void) 
{
  	ast *left, *right, *tree;
  	int id;

  	//check for identifier
  	ident();

  	//check it's been defined then make a leaf node for it
  	if((id = findglob(Text)) == -1) 
	{
    		fatals("Undeclared variable", Text);
  	}
  	right = mkastleaf(A_LVIDENT, id);

  	//ensure we have an equals sign
  	match(T_ASSIGN, "=");

  	//parse the following expression
  	left = binexpr(0);

  	//make an assignment AST tree
  	tree = mkastnode(A_ASSIGN, left, NULL, right, 0);

	//return the tree..
  	return tree;
}

// if_statement: if_head
//      |        if_head 'else' compound_statement
//      ;
//
// if_head: 'if' '(' true_false_expression ')' compound_statement  ;
//
// Parse an IF statement including
// any optional ELSE clause
// and return its AST
ast *if_statement(void) 
{
  	ast *condAST, *trueAST, *falseAST = NULL;

  	//we need first'if' and '('
  	match(T_IF, "if");
  	lparen();

  	//parse the following expression and the ')' following, ensure the tree's operation is a comparison.
  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		fatal("Bad comparison operator");
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
  	return (mkastnode(A_IF, condAST, trueAST, falseAST, 0));
}


// while_statement: 'while' '(' true_false_expression ')' compound_statement  ;
//
//parse a WHILE statement and return its AST
ast *while_statement(void) 
{
  	ast *condAST, *bodyAST;

  	//check that we have 'while' '('
  	match(T_WHILE, "while");
  	lparen();

  	//parse the following expression and the ')' following. ensure the tree's operation is a comparison.
  	condAST = binexpr(0);
  	if(condAST->op < A_EQ || condAST->op > A_GE)
    		fatal("Bad comparison operator");

  	rparen();

  	//get the AST for the compound statement
  	bodyAST = compound_statement();

  	//build and return the AST for this statement
	return mkastnode(A_WHILE, condAST, NULL, bodyAST, 0);
}

static ast *for_statement(void)
{
	ast *condAST, *bodyAST;
	ast *preopAST, *postopAST;
	ast *tree;

	//check that we have for '('
	match(T_FOR, "for");
	lparen();

	//get the preop statement and ;
	preopAST = single_statement();

	semi();

	//get the condition and ;
	condAST = binexpr(0);
	if(condAST -> op < A_EQ || condAST -> op > A_GE)
		fatal("Bad comparison operator");
	
	semi();

	postopAST = single_statement();
	rparen();

	//get the body of for loop in bodyAST..
	bodyAST = compound_statement();

	tree = mkastnode(A_GLUE, bodyAST, NULL, postopAST, 0); //glue comp_Stmt with postop tree.
	
	//make while loopp with condition with this new block of code..
	tree = mkastnode(A_WHILE, condAST, NULL, tree, 0);

	return mkastnode(A_GLUE, preopAST, NULL, tree, 0); //glue preop tree with A_WHILE treee
}

//parse single_stmt and return tree(AST) of it..

ast *single_statement(void)
{
	switch(Token.token)
	{
		case T_PRINT:
			return print_statement();
		case T_INT:
			var_declaration();
			return NULL;
		case T_IDENT:
			return assignment_statement();
		case T_IF:
			return if_statement();
		case T_WHILE:
			return while_statement();
		case T_FOR:
			return for_statement();
		default :
			fatald("Syntax error, token", Token.token);
	}
}

//parse a compound statement and return its AST Tree node..
ast *compound_statement(void) 
{
  	ast *left = NULL;
  	ast *tree;

  	//there should be left curly bracket..
  	lbrace();

  	while(1)
  	{
		
		tree = single_statement(); //parse sing_stmt;
		
		if(tree != NULL && (tree -> op == A_PRINT || tree -> op == A_ASSIGN))
			semi();

    		//for every new tree, either save it in left, if left is empty, or glue the left tree
		//with the new tree.. 

	    	if(tree)
    		{
      			if(left == NULL)
				left = tree;
      			else
				left = mkastnode(A_GLUE, left, NULL, tree, 0);
    		}

		//if we hit right curly then just return the AST..
		if(Token.token == T_RBRACE)
		{
			rbrace();
			return left;
		}
  	}
}
