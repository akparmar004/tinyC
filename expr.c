#include "defs.h"
#include "data.h"
#include "decl.h"

//parsing expressions

//parse function call with a single expression argument and return its AST
static ast *funccall(void) 
{
  	ast *tree;
  	int id;

  	//check that identifier has been defined as a function, then make a leaf node for it.
  	if((id = findglob(Text)) == -1 || Gsym[id].stype != S_FUNCTION) 
  	{
    		fatals("Undeclared function", Text);
  	}
  	
	//scan '('
  	lparen();

  	//parse the following expression(argument)
  	tree = binexpr(0);

  	//build the function call AST node.store the function's return type as this node's type.
  	//and also record the function's symbol-id
  	tree = mkastunary(A_FUNCCALL, Gsym[id].type, tree, id);

  	//scan ')'
  	rparen();
  	return tree;
}

//parse the index into an array and return an AST tree for it
static ast *array_access(void) 
{
  	ast *left, *right;
  	int id;

  	//check that the identifier has been defined as an array
  	//then make a leaf node for it that points at the base
  	if((id = findglob(Text)) == -1 || Gsym[id].stype != S_ARRAY) 
	{
    		fatals("Undeclared array", Text);
  	}
  	left = mkastleaf(A_ADDR, Gsym[id].type, id);

  	//scan '['
  	scan(&Token);

  	//parse the following expression
  	right = binexpr(0);

  	//then scan ']'
  	match(T_RBRACKET, "]");

  	//check that this is int type
  	if(!inttype(right->type))
    		fatal("Array index is not of integer type");

  	//scale the index by the size of the element's type
  	right = modify_type(right, left->type, A_ADD);

	//return AST tree where the array's base has the offset added to it,
  	//and dereference element, still lvalue at this point...
  	left = mkastnode(A_ADD, Gsym[id].type, left, NULL, right, 0);
  	left = mkastunary(A_DEREF, value_at(left->type), left, 0);
  	
	return left;
}

//parse a primary factor and return "ast" node representing it.
static ast *primary(void) 
{
  	ast *n;
  	int id;

	switch(Token.token) 
	{
  		case T_INTLIT:
    			//for INTLIT token, make a leaf AST node for it.
    			//make it a P_CHAR if it's within the P_CHAR range
    			if ((Token.intvalue) >= 0 && (Token.intvalue < 256))
      				n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
    			else
      				n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
    			break;
  		case T_IDENT:
    			//this could be a var, array index or a function call.
			//scan in the next token to find out
    			scan(&Token);

    			//if it's '(', so a function call
    			if(Token.token == T_LPAREN)
      				return funccall();

    			//if it's a '[', so an array reference
    			if(Token.token == T_LBRACKET) 
      				return array_access();
    			
    			//reject new token if it's not a func call..
    			reject_token(&Token);

    			//check that the variable exists or not...
    			id = findglob(Text);
    			if(id == -1 || Gsym[id].stype != S_VARIABLE)
      				fatals("Unknown variable", Text);

    			//creat a leaf AST node for it
    			n = mkastleaf(A_IDENT, Gsym[id].type, id);
    			break;

  		case T_LPAREN:
    			//beginning of a parenthesised expression, skip the '('.
    			//scan expression and the right parenthesis
    			scan(&Token);
    			n = binexpr(0);
    			rparen();
    			
			return n;

  		default:
    			fatald("Expecting a primary expression, got token", Token.token);
  	}

  	//scan in the next token and return the leaf node
  	scan(&Token);
  	return n;
}


//convert a binary operator token into a binary AST operation...
//we rely on a 1:1 convirtion from token to AST operation 
static int binastop(int tokentype) 
{
  	if(tokentype > T_EOF && tokentype < T_INTLIT)
    		return tokentype;
  	
	fatald("Syntax error, token", tokentype);
  	return 0;
}

//return true if a token is right-associative, else false...
static int rightassoc(int tokentype) 
{
  	if(tokentype == T_ASSIGN)
    		return 1;
  	
	return 0;
}

//operator precedence for each token, must match up with the order of tokens in defs.h
static int OpPrec[] = 
{
  	0, 10,			// T_EOF,  T_ASSIGN
  	20, 20,			// T_PLUS, T_MINUS
  	30, 30,			// T_STAR, T_SLASH
  	40, 40,			// T_EQ, T_NE
  	50, 50, 50, 50		// T_LT, T_GT, T_LE, T_GE
};

//check that we have a binary operator and return its precedence.
static int op_precedence(int tokentype) 
{
  	int prec;
  	if(tokentype >= T_VOID)
    		fatald("Token with no precedence in op_precedence:", tokentype);
  	
	prec = OpPrec[tokentype];
  	
	if (prec == 0)
    		fatald("Syntax error, token", tokentype);
  	
	return prec;
}

//prefix_expression: primary | '*' prefix_expression | '&' prefix_expression ;

//parse prefix expression and return a treee...
ast *prefix(void) 
{
  	ast *tree;
  	
	switch(Token.token) 
	{
  		case T_AMPER:
    			//get the next token and parse it recursively as a prefix expression
    			scan(&Token);
    			tree = prefix();

    			//check that it's an identifier
    			if(tree->op != A_IDENT)
      				fatal("& operator must be followed by an identifier");

    			//now change the operator to A_ADDR and the type to a pointer to the original type
    			tree->op = A_ADDR;
    			tree->type = pointer_to(tree->type);
    			break;
  		case T_STAR:
    			//get the next token and parse it recursively as a prefix expression
    			scan(&Token);
    			tree = prefix();

    			//for now, ensure it's either another deref or an identifier
    			if(tree->op != A_IDENT && tree->op != A_DEREF)
      				fatal("* operator must be followed by an identifier or *");

    			//prepend an A_DEREF operation to the tree
    			tree = mkastunary(A_DEREF, value_at(tree->type), tree, 0);
    			break;
  		default:
    			tree = primary();
  	}
  	return tree;
}

//return an AST tree whose root is a binary operator, parameter ptp is the previous token's precedence.
ast *binexpr(int ptp) 
{
  	ast *left, *right;
  	ast *ltemp, *rtemp;
  	int ASTop;
  	int tokentype;

  	//get the tree on the left, fetch the next token at the same time.
  	left = prefix();

  	//if we hit a semicolon or ')', return just the left node
  	tokentype = Token.token;
  	if(tokentype == T_SEMI || tokentype == T_RPAREN || tokentype == T_RBRACKET) 
	{
    		left->rvalue = 1;
    		return left;
  	}
  	
	//while the precedence of this token is more than that of the 
	//previous token precedence, or it's right associative and equal to the previous token's precedence
  	while((op_precedence(tokentype) > ptp) || (rightassoc(tokentype) && op_precedence(tokentype) == ptp)) 
	{
    		//fetch in the next integer literal
    		scan(&Token);

    		//recursively call binexpr() with the precedence of our token to build a sub-tree
    		right = binexpr(OpPrec[tokentype]);

    		//determine the operation to be performed on the sub-trees
    		ASTop = binastop(tokentype);

    		if(ASTop == A_ASSIGN) 
    		{
      			//assignment make the right tree into an rvalue
      			right->rvalue = 1;

      			//ensure the right's type matches the left
      			right = modify_type(right, left->type, 0);
      			if(left == NULL)
				fatal("Incompatible expression in assignment");

      			//make an assignment AST tree. However, switch
      			//left and right around, so that the right expression's 
      			//code will be generated before the left expression
      			ltemp = left;
      			left = right;
      			right = ltemp;
    		}
	       	else 
		{
      			//we are not doing an assignment, so both trees should be rvalues
      			//convert both trees into rvalue if they are lvalue trees
      			left -> rvalue = 1;
      			right -> rvalue = 1;

      			//ensure the two types are compatible by trying
      			// to modify each tree to match the other's type.
      			ltemp = modify_type(left, right->type, ASTop);
      			rtemp = modify_type(right, left->type, ASTop);

      			if(ltemp == NULL && rtemp == NULL)
				fatal("Incompatible types in binary expression");
      			
			if(ltemp != NULL)
				left = ltemp;
      			
			if(rtemp != NULL)
				right = rtemp;
    		}

    		//join that sub-tree with ours. Convert the token into an AST operation at the same time.
    		left = mkastnode(binastop(tokentype), left->type, left, NULL, right, 0);

    		//update the details of the current token.
    		//if we hit a semicolon, ')' or ']', return just the left node
    		tokentype = Token.token;
    		if (tokentype == T_SEMI || tokentype == T_RPAREN || tokentype == T_RBRACKET) 
		{
      			left -> rvalue = 1;
      			return left;
    		}
  	}

  	//return the tree we have when the precedence is the same or lower
  	left -> rvalue = 1;
  	return left;
}
