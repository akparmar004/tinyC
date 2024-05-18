#include "defs.h" 
#include "data.h"
#include "decl.h"     

//parsing of statements

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

  	//match the following semicolon and return the AST
  	semi();
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

  	//match the following semicolon and return the AST
  	semi();
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
struct ASTnode *if_statement(void) 
{
  	struct ASTnode *condAST, *trueAST, *falseAST = NULL;

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
// Parse a WHILE statement
// and return its AST
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


//parse a compound statement and return its AST Tree node..
ast *compound_statement(void) 
{
  	ast *left = NULL;
  	ast *tree;

  	//there should be left curly bracket..
  	lbrace();

  	while(1)
  	{
    		switch(Token.token) 
		{
      			case T_PRINT:
				tree = print_statement();
				break;
      			case T_INT:
				var_declaration();
				tree = NULL;		//no AST generated here
				break;
      			case T_IDENT:
				tree = assignment_statement();
				break;
      			case T_IF:
				tree = if_statement();
				break;
      			case T_WHILE:
				tree = while_statement();
				break;
      			case T_RBRACE:
				//when we hit a right curly bracket, skip past it and return the ast
				rbrace();
				return (left);
		        default:
				fatald("Syntax error, token", Token.token);
    		}

    		//for every new tree, either save it in left, if left is empty, or glue the left tree
		//with the new tree.. 
	    	if(tree)
    		{
      			if(left == NULL)
				left = tree;
      			else
				left = mkastnode(A_GLUE, left, NULL, tree, 0);
    		}
  	}
}
