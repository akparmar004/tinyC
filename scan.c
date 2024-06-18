#include "defs.h"
#include "data.h"
#include "decl.h"

//lexical scanner

//return position of char c in string s or -1 if c not available..
static int chrpos(char *s, int c) 
{
  	char *p;

  	p = strchr(s, c);
  	return (p ? p - s : -1);
}

//get the next char from the input file... with fgetc..
static int next(void)
{
  	int c;

  	if(Putback)
       	{				
    		c = Putback;		//use putback if needed
    		Putback = 0;
    		return c;
  	}

  	c = fgetc(Infile);		//read char from input file
  	if('\n' == c)
    		Line++;			//if it's "\n" then increment line count
  	
	return c;
}

//put back an unwanted character
static void putback(int c) 
{
  	Putback = c;
}

//skip chars like i.e. whitespace, newlines,  and return the first character we do need to deal with...
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

static int scanch(void)
{
	int c;

	c = next();
	if(c == '\\')
	{
		switch(c = next())
		{
			case 'a':
				return '\a';
			case 'b':
				return '\b';
			case 'f':
				return '\f';
			case 'n':
				return '\n';
			case 'r':
				return '\r';
			case 't':
				return '\t';
			case 'v':
				return '\v';
			case '\\':
				return '\\';
			case '"':
				return '"';
			case '\'':
				return '\'';
			default :
				fatalc("unknown escape sequence", c);
		}
	}
	return c;
}


//scan and return an int lit value 
static int scanint(int c) 
{
  	int k, val = 0;

  	//convert each character into an int value using our chrpos func..
  	while((k = chrpos("0123456789", c)) >= 0) 
  	{
    		val = val * 10 + k;
    		c = next();
  	}

  	//we hit a non-int char, put it back at its place..
  	putback(c);
  	return val;
}


static int scanstr(char *buf)
{
	int i, c;

	for(i = 0; i < TEXTLEN-1; i++)
	{
		if((c = scanch()) == '"')
		{
			buf[i] = 0;
			return i;
		}

		buf[i] = c;
	}
	fatal("string literal too long");
	return 0;
}

//scan ident and store it in buf[].. and return length of ident..
static int scanident(int c, char *buf, int lim) 
{
  	int i = 0;

  	//only allow digits, alpha and underscores
  	while(isalpha(c) || isdigit(c) || '_' == c) 
	{
    		if(lim - 1 == i) 
		{
      			fatal("Identifier too long");
    		} 
		else if (i < lim - 1) 
		{
      			buf[i++] = c;
    		}
    		c = next();
  	}
  	
	//we hit a non-valid char, put it back, and terminate the buf[] with '\0'
  	putback(c);
  	buf[i] = '\0';
  	return i;
}

//match a keyword.. if  not matching erturn 0 else ret token..
static int keyword(char *s) 
{
  	switch(*s) 
	{
    		case 'c':
      			if (!strcmp(s, "char"))
				return T_CHAR;
      			break;
    		case 'e':
      			if(!strcmp(s, "else"))
				return T_ELSE;
      			break;
    		case 'f':
      			if(!strcmp(s, "for"))
				return T_FOR;
      			break;
    		case 'i':
      			if(!strcmp(s, "if"))
				return T_IF;
      			if(!strcmp(s, "int"))
				return T_INT;
      			break;
    		case 'l':
      			if(!strcmp(s, "long"))
				return T_LONG;
      			break;
    		case 'r':
      			if(!strcmp(s, "return"))
				return T_RETURN;
      			break;
    		case 'w':
      			if(!strcmp(s, "while"))
				return T_WHILE;
      			break;
    		case 'v':
      			if(!strcmp(s, "void"))
				return T_VOID;
      			break;
  	}
  	return 0;
}

//a pointer to a rejected token
static struct token *Rejtoken = NULL;

//reject token that we just scanned
void reject_token(struct token *t) 
{
  	if(Rejtoken != NULL)
    		fatal("Can't reject token twice");
  	
	Rejtoken = t;
}

//scan and return token ..
int scan(struct token *t) 
{
  	int c, tokentype;

  	//if we have any rejected token, return it
  	if(Rejtoken != NULL) 
	{
    		t = Rejtoken;
    		Rejtoken = NULL;
    		return 1;
  	}
  	
	//skip whitespace
  	c = skip();

  	//determine the token based on the input character
  	switch(c) 
  	{
    		case EOF:
      			t->token = T_EOF;
      			return 0;
    		case '+':
      			if((c = next()) == '+') 
      			{
				t -> token = T_INC;
      			}
      			else 
      			{
				putback(c);
				t -> token = T_PLUS;
      			}
      			break;
    		case '-':
      			if((c = next()) == '-') 
      			{
				t -> token = T_DEC;
      			} 
      			else 
      			{
				putback(c);
				t -> token = T_MINUS;
      			}
      			break;
    		case '*':
      			t->token = T_STAR;
      			break;
    		case '/':
      			t->token = T_SLASH;
      			break;
    		case ';':
      			t->token = T_SEMI;
      			break;
    		case '{':
      			t->token = T_LBRACE;
      			break;
    		case '}':
      			t->token = T_RBRACE;
      			break;	
    		case '(':
      			t->token = T_LPAREN;
      			break;
    		case ')':
      			t->token = T_RPAREN;
      			break;
    		case '[':
      			t->token = T_LBRACKET;
      			break;
    		case ']':
      			t->token = T_RBRACKET;
      			break;
      		case '~':
      			t -> token = T_INVERT;
      			break;
    		case '^':
      			t -> token = T_XOR;
      			break;
    		case ',':
      			t -> token = T_COMMA;
      			break;
    		case '=':
      			if((c = next()) == '=') 
      			{
				t->token = T_EQ;
      			}
      			else
      			{
				putback(c);
				t->token = T_ASSIGN;
      			}
      			break;	
    		case '!':
      			if((c = next()) == '=') 
      			{
				t->token = T_NE;
      			}
      			else 
      			{
				putback(c);
				t -> token = T_LOGNOT;
      			}
      			break;
    		case '<':
      			if((c = next()) == '=') 
      			{
				t->token = T_LE;
      			}
      			else if(c == '<') 
      			{
				t->token = T_LSHIFT;
      			}  
      			else
      			{
				putback(c);
				t->token = T_LT;
      			}
      			break;
    		case '>':
      			if((c = next()) == '=') 
      			{
				t -> token = T_GE;
      			} 
      			else if(c == '>') 
      			{
				t -> token = T_RSHIFT;
      			} 
      			else 
      			{
				putback(c);
				t -> token = T_GT;
      			}
      			break;
    		case '&':
      			if((c = next()) == '&') 
      			{
				t -> token = T_LOGAND;
      			} 
      			else 
      			{
				putback(c);
				t -> token = T_AMPER;
      			}
      			break;
      		case '|':
      			if((c = next()) == '|') 
      			{
				t -> token = T_LOGOR;
      			} 
      			else 
      			{
				putback(c);
				t -> token = T_OR;
      			}
      			break;
		case '\'':
			t -> intvalue = scanch();
			t -> token = T_INTLIT;
			if(next() != '\'')
				fatal("Expected '\\'' at end of char literal");
			break;
		case '"':
			scanstr(Text);
			t -> token = T_STRLIT;
			break;
    		default:
      			//if it's a digit, scan int lit value in
      			if(isdigit(c)) 
      			{
				t -> intvalue = scanint(c);
				t -> token = T_INTLIT;
				break;
      			} 
      			else if (isalpha(c) || '_' == c) 
      			{
				//read that word or ident
				scanident(c, Text, TEXTLEN);

				//if it's a recognised keyword, then ret..
				if((tokentype = keyword(Text)) != 0) 
				{
	  				t -> token = tokentype;
	  				break;
				}
				//not a recognised keyword, so it must be an identifier
				t -> token = T_IDENT;
				break;
      			}
      			//the character isn't part of any recognised token, error
      			fatalc("Unrecognised character", c);
  		}	

  	//return with success
  	return (1);
}
