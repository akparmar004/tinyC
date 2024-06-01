#include "defs.h"
#include "data.h"
#include "decl.h"

//parsing of declarations

//pass the current token and return a primitive type enum value and also scan in the next token
int parse_type(void) 
{
  	int type;
  	switch(Token.token) 
	{
    		case T_VOID:
      			type = P_VOID;
      			break;
    		case T_CHAR:
      			type = P_CHAR;
      			break;
    		case T_INT:
      			type = P_INT;
      			break;
    		case T_LONG:
      			type = P_LONG;
      			break;
    		default:
      			fatald("Illegal type, token", Token.token);
  	}

  	//scan in one or more further '*' tokens and determine the correct pointer type
  	while (1) 
  	{
    		scan(&Token);
    		if(Token.token != T_STAR)
      			break;
    		
		type = pointer_to(type);
  	}

  	//we will return with next token in "Token"..
  	return type;
}

// variable_declaration: type identifier ';' | type identifier '[' INTLIT ']' ';' ;

//parse the declaration of var or array with a given size...
//the identifier has been scanned & we have the type
void var_declaration(int type) 
{
  	int id;

  	//we have ident's name in Text, if the next token is a '['
  	if(Token.token == T_LBRACKET) 
	{
    		//skip past the '['
    		scan(&Token);

    		//check that we have an array size
    		if(Token.token == T_INTLIT) 
		{
      			//add this as known array and generate its space in assembly.
      			//we treat the array as a pointer to its elements' type
      			id = addglob(Text, pointer_to(type), S_ARRAY, 0, Token.intvalue);
      			genglobsym(id);
    		}

    		//check that we have a following ']'
    		scan(&Token);
    		match(T_RBRACKET, "]");
  	} 
	else 
	{
    		//add this as a known ident.. and generate its space in assembly
    		id = addglob(Text, type, S_VARIABLE, 0, 1);
    		genglobsym(id);
  	}

  	//scan semicolon
  	semi();
}

//function_declaration: type identifier '(' ')' compound_statement 

//parse the declaration of a simplistic function, and The identifier has been scanned & we have the type
ast *function_declaration(int type) 
{
  	ast *tree, *finalstmt;
  	int nameslot, endlabel;

  	//text now has the identifier's name, get a label-id for the end label, 
	//add the function to the symbol table, and set the Functionid global to the function's symbol-id
  	endlabel = genlabel();
  	nameslot = addglob(Text, type, S_FUNCTION, endlabel, 0);
  	Functionid = nameslot;

  	//scan in the parentheses
  	lparen();
  	rparen();

  	//get the AST tree for the compound statement
  	tree = compound_statement();

  	//if the function type isn't P_VOID ..
  	if(type != P_VOID) 
	{
		//error if no statements in the function
    		if(tree == NULL)
      			fatal("No statements in function with non-void type");

    		//check that the last AST operation in the compound statement was a return statement
    		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;

    		if(finalstmt == NULL || finalstmt->op != A_RETURN)
      		fatal("No return for function with non-void type");
  	}

  	//return A_FUNCTION
  	return mkastunary(A_FUNCTION, type, tree, nameslot);
}


//parse one or more global declarations, either variables or functions
void global_declarations(void) 
{
  	ast *tree;
  	int type;

  	while(1) 
	{
    		//we have to read past the type and identifier to see either a '(' for a function declaration
    		//or a ',' or ';' for a variable declaration.
    		//text is filled in by the ident() call.
    		type = parse_type();
    		ident();

    		if(Token.token == T_LPAREN) 
		{
			//parse the function declaration and generate the assembly code..
       			tree = function_declaration(type);
       			
			if(O_dumpAST) 
			{ 
				dumpAST(tree, NOLABEL, 0); fprintf(stdout, "\n\n"); 
			}
       			genAST(tree, NOLABEL, 0);
    		} 
		else 
		{
       			//parse the global variable declaration
       			var_declaration(type);
    		}

    		//stop when we have reached EOF
    		if(Token.token == T_EOF)
      			break;
  	}
}
