#include "defs.h"
#include "data.h"
#include "decl.h"

//********************* lexical scanning *********************

//return the position of character c in string s, or return -1 if that char not found simply..
static int chrpos(char *s, int c) 
{
  	char *p;

  	p = strchr(s, c);
  	return (p ? p - s : -1);
}

//get the next character from the input file.
static int next(void) 
{
  	int c;

  	if(Putback) 
	{		
		//use the character put back if there is one
    		c = Putback
    		Putback = 0;
    		return c;
  	}

  	c = fgetc(Infile);	//read the char from input file
  	if('\n' == c)
    		Line++;		//increment line count for \n
  	return c;
}

//put back an unwanted character
static void putback(int c) 
{
  	Putback = c;
}

//skip past input that we don't need to deal with, i.e. whitespace, newlines.
//return the first character we do need to deal with.
static int skip(void) 
{
  	int c;

  	c = next();
  	while(' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c) 
	{
    		c = next();
  	}
  	return c;
}

//scan the whole integer literal value from file and return it..
static int scanint(int c) 
{
  	int k, val = 0;

  	//convert each character into an int value
  	while((k = chrpos("0123456789", c)) >= 0) 
	{
    		val = val * 10 + k;
   		c = next();
  	}

  	//we hit a non-integer character, put it back.
	putback(c);
  	return (val);
}

//scan an identifier from the input file and store it in buf[], and also return the identifier's length
static int scanident(int c, char *buf, int lim) 
{
  	int i = 0;

  	//digits, alpha and underscores is allowed.. for identifiers..
  	while (isalpha(c) || isdigit(c) || '_' == c) 
  	{
    		//error if we hit the identifier length limit, else append to buf[] and get next character
		if(lim - 1 == i) 
		{
      			fatal("Identifier too long");
	        } 
		else if(i < lim - 1) 
		{
      			buf[i++] = c;
    		}
    		c = next();
  	}
  	//we hit a non-valid character, put it back at it's place.terminate the buf[] with null and return the length
  	putback(c);
  	buf[i] = '\0';
  	return (i);
}

//given a word from input, return the matching keyword token number or 0 if it's not a keyword.
//we will use strcmp() to compare the whole word.. because we dont wanna waste our time into every characters..
static int keyword(char *s) 
{
  	switch(*s) 
	{
    		case 'e':
      			if(!strcmp(s, "else"))
				return (T_ELSE);
      			break;
    		case 'i':
      			if(!strcmp(s, "if"))
				return (T_IF);
      			if(!strcmp(s, "int"))
				return (T_INT);
      			break;
    		case 'p':
      			if(!strcmp(s, "print"))
				return (T_PRINT);
      			break;
    		case 'w':
      			if(!strcmp(s, "while"))
				return (T_WHILE);
      			break;
  	}
  	return (0);
}

//scan and return the next token found in the input, return 1 if token valid, 0 if no tokens left.
int scan(struct token *t) {
  	int c, tokentype;

  	//skip whitespace or escape sequence chars..
	c = skip();

  	//fetch token based on input char..
	switch (c)
  	{
    		case EOF:
	        	t -> token = T_EOF;
      			return (0);
    		case '+':
      			t -> token = T_PLUS;
      			break;
    		case '-':
      			t -> token = T_MINUS;
      			break;
    		case '*':
      			t -> token = T_STAR;
      			break;
    		case '/':
      			t -> token = T_SLASH;
      			break;
    		case ';':
      			t -> token = T_SEMI;
      			break;
    		case '{':
      			t -> token = T_LBRACE;
      			break;
    		case '}':
      			t -> token = T_RBRACE;
      			break;
    		case '(':
      			t -> token = T_LPAREN;
      			break;
    		case ')':
      			t -> token = T_RPAREN;
      			break;
    		case '=':
      			if ((c = next()) == '=') 
			{
				t -> token = T_EQ;
		        }
		       	else 
			{	
				putback(c);
				t -> token = T_ASSIGN;
      			}
      			break;
    		case '!':
      			if ((c = next()) == '=') 
			{
				t -> token = T_NE;
      			}
		       	else 
			{
				fatalc("Unrecognised character", c);
			}
      			break;
    		case '<':
	   		if ((c = next()) == '=') 
			{
				t -> token = T_LE;
      			} 
			else 
			{
				putback(c);
				t -> token = T_LT;
      			}
      			break;
    		case '>':
		        if ((c = next()) == '=') 
			{
				t -> token = T_GE;
      			}
		       	else 
			{
				putback(c);
				t -> token = T_GT;
  			}
      			break;
    		default :
	                //if it's a digit, scan the literal integer value in
		        if(isdigit(c)) 
			{
				t -> intvalue = scanint(c);
				t -> token = T_INTLIT;
			break;
      			}
		       	else if(isalpha(c) || '_' == c) 
			{
				//scan a keyword or identifier
				scanident(c, Text, TEXTLEN);
				
				//if it's a recognised keyword, return that token
				if(tokentype = keyword(Text)) 
				{
	  				t -> token = tokentype;
	  				break;
				}
				
				//so now, it must be an identifier
				t -> token = T_IDENT;
				break;
      			}
      			//the character isn't part of any recognised token, error
      			fatalc("Unrecognised character", c);
  		}

  	//we have our token now..
  	return 1;
}
