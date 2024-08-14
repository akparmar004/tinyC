#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//lexical Scaner

static int chrpos(char *s, int c) 
{
  	int i;
  	for(i = 0; s[i] != '\0'; i++)
	{
    		if(s[i] == (char) c)
      			return (i);
	}
  	return (-1);
}

static int next(void) 
{
  	int c, l;

  	if(Putback) 
  	{				
    		c = Putback;		//back if there is one
    		Putback = 0;
    		
    		return (c);
  	}

  	c = fgetc(Infile);		//read from input file

  	while(Linestart && c == '#') 
  	{				
    		Linestart = 0;		
    		scan(&Token);		
    		if (Token.token != T_INTLIT)
      			fatals("Expecting pre-processor line number, got:", Text);
    		l = Token.intvalue;

    		scan(&Token);		
    		if (Token.token != T_STRLIT)
      			fatals("Expecting pre-processor file name, got:", Text);

    		if (Text[0] != '<') 
    		{				
      			if (strcmp(Text, Infilename))		
				Infilename = strdup(Text);	
      			
      			Line = l;
    		}

    		while((c = fgetc(Infile)) != '\n');	
    			
		c = fgetc(Infile);			
    		Linestart = 1;				
  	}

  	Linestart = 0;		
  	if('\n' == c) 
  	{
    		Line++;			
    		Linestart = 1;		
  	}
  	return c;
}

static void putback(int c) 
{
  	Putback = c;
}

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

static int hexchar(void) 
{
  	int c, h, n = 0, f = 0;

  	while(isxdigit(c = next())) 
  	{
    		h = chrpos("0123456789abcdef", tolower(c));
    		
    		n = n * 16 + h;
    		f = 1;
  	}
  	putback(c);
  	if (!f)
    		fatal("missing digits after '\\x'");
  	
  	if(n > 255)
    		fatal("value out of range after '\\x'");
  	
  	return n;
}

static int scanch(void) 
{
  	int i, c, c2;

  	c = next();
  	if(c == '\\') 
  	{
    		switch(c = next()) 
    		{
      			case 'a':
				return ('\a');
      			case 'b':
				return ('\b');
      			case 'f':
				return ('\f');
      			case 'n':
				return ('\n');
      			case 'r':
				return ('\r');
      			case 't':
				return ('\t');
      			case 'v':
				return ('\v');
      			case '\\':
				return ('\\');
      			case '"':
				return ('"');
      			case '\'':
				return ('\'');
      			case '0':
      			case '1':
      			case '2':
      			case '3':
      			case '4':
      			case '5':
      			case '6':
      			case '7':
				for(i = c2 = 0; isdigit(c) && c < '8'; c = next()) 
				{
	  				if(++i > 3)
	    					break;
	  				c2 = c2 * 8 + (c - '0');
				}
				putback(c);		
				return (c2);
      			case 'x':
				return (hexchar());
      			default:
				fatalc("unknown escape sequence", c);
    		}
  	}
  	return c;			
}

static int scanint(int c) 
{
  	int k, val = 0, radix = 10;

  	if(c == '0') 
  	{
    		if((c = next()) == 'x') 
    		{
      			radix = 16;
      			c = next();
    		} 
    		else
      			radix = 8;
  	}
  	while((k = chrpos("0123456789abcdef", tolower(c))) >= 0) 
  	{
    		if(k >= radix)
      			fatalc("invalid digit in integer literal", c);
    		
    		val = val * radix + k;
    		c = next();
  	}
  	
	putback(c);
  	return val;
}

static int scanstr(char *buf) 
{
  	int i, c;

  	for(i = 0; i < TEXTLEN - 1; i++) 
  	{
    		if((c = scanch()) == '"') 
    		{
      			buf[i] = 0;
      			return (i);
    		}
    		buf[i] = (char)c;
  	}
  	fatal("String literal too long");

  	return 0;
}

static int scanident(int c, char *buf, int lim) 
{
  	int i = 0;

  	while(isalpha(c) || isdigit(c) || '_' == c) 
  	{
    		if (lim - 1 == i) 
    		{
      			fatal("Identifier too long");
    		} 
    		else if (i < lim - 1) 
    		{
      			buf[i++] = (char)c;
    		}
    		c = next();
  	}
  	putback(c);
  	buf[i] = '\0';
  	
  	return i;
}

static int keyword(char *s) 
{
  	switch (*s) 
  	{
    		case 'b':
      			if(!strcmp(s, "break"))
				return (T_BREAK);
      			break;
    		case 'c':
      			if (!strcmp(s, "case"))
				return (T_CASE);
      			if (!strcmp(s, "char"))
				return (T_CHAR);
     			if (!strcmp(s, "continue"))
				return (T_CONTINUE);
      			break;
    		case 'd':
      			if (!strcmp(s, "default"))
				return (T_DEFAULT);
      			break;
    		case 'e':
      			if (!strcmp(s, "else"))
				return (T_ELSE);
      			if (!strcmp(s, "enum"))
				return (T_ENUM);
      			if (!strcmp(s, "extern"))
				return (T_EXTERN);
      			break;
    		case 'f':
      			if (!strcmp(s, "for"))
				return (T_FOR);
      			break;
    		case 'i':
      			if (!strcmp(s, "if"))
				return (T_IF);
      			if(!strcmp(s, "int"))
				return (T_INT);
      			break;
    		case 'l':
      			if (!strcmp(s, "long"))
				return (T_LONG);
      			break;
    		case 'r':
      			if (!strcmp(s, "return"))
				return (T_RETURN);
      			break;
    		case 's':
      			if (!strcmp(s, "sizeof"))
				return (T_SIZEOF);
      			if (!strcmp(s, "static"))
				return (T_STATIC);
      			if (!strcmp(s, "struct"))
				return (T_STRUCT);
      			if (!strcmp(s, "switch"))
				return (T_SWITCH);
      			break;
    		case 't':
      			if (!strcmp(s, "typedef"))
				return (T_TYPEDEF);
      			break;
    		case 'u':
      			if (!strcmp(s, "union"))
				return (T_UNION);
      			break;
   		case 'v':
      			if (!strcmp(s, "void"))
				return (T_VOID);
     			break;
    		case 'w':
      			if (!strcmp(s, "while"))
				return (T_WHILE);
      			break;
  	}
  	return 0;
}

char *Tstring[] = 
{
  "EOF", "=", "+=", "-=", "*=", "/=", "%=",
  "?", "||", "&&", "|", "^", "&",
  "==", "!=", ",", ">", "<=", ">=", "<<", ">>",
  "+", "-", "*", "/", "%", "++", "--", "~", "!",
  "void", "char", "int", "long",
  "if", "else", "while", "for", "return",
  "struct", "union", "enum", "typedef",
  "extern", "break", "continue", "switch",
  "case", "default", "sizeof", "static",
  "intlit", "strlit", ";", "identifier",
  "{", "}", "(", ")", "[", "]", ",", ".",
  "->", ":"
};

int scan(struct token *t) 
{
  	int c, tokentype;

  	if(Peektoken.token != 0) 
  	{
    		t->token = Peektoken.token;
    		t->tokstr = Peektoken.tokstr;
    		t->intvalue = Peektoken.intvalue;
    		Peektoken.token = 0;
    		
		return 1;
  	}
  	c = skip();

  	switch(c) 
  	{
    		case EOF:
      			t->token = T_EOF;
      			return (0);
    		case '+':
      			if((c = next()) == '+') 
      			{
				t->token = T_INC;
      			} 
      			else if(c == '=') 
      			{
				t->token = T_ASPLUS;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_PLUS;
      			}
      			break;
    		case '-':
      			if((c = next()) == '-') 
      			{
				t->token = T_DEC;
      			} 
      			else if(c == '>') 
      			{
				t->token = T_ARROW;
      			} 
      			else if(c == '=') 
      			{
				t->token = T_ASMINUS;
      			} 
      			else if (isdigit(c)) 
      			{	
      				//negative int literal
				t->intvalue = -scanint(c);
				t->token = T_INTLIT;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_MINUS;
      			}
      			break;
   		case '*':
      			if ((c = next()) == '=') 
      			{
				t->token = T_ASSTAR;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_STAR;
      			}
      			break;
    		case '/':
      			if((c = next()) == '=') 
      			{
				t->token = T_ASSLASH;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_SLASH;
      			}
      			break;
    		case '%':
      			if ((c = next()) == '=') 
      			{
				t->token = T_ASMOD;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_MOD;
      			}
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
      			t->token = T_INVERT;
      			break;
    		case '^':
      			t->token = T_XOR;
      			break;
    		case ',':
      			t->token = T_COMMA;
      			break;
    		case '.':
      			t->token = T_DOT;
      			break;
    		case ':':
      			t->token = T_COLON;
      			break;
    		case '?':
      			t->token = T_QUESTION;
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
      			if ((c = next()) == '=') 
      			{
				t->token = T_NE;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_LOGNOT;
      			}
      			break;
    		case '<':
      			if((c = next()) == '=') 
      			{
				t->token = T_LE;
      			} 
      			else if (c == '<') 
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
				t->token = T_GE;
      			} 
      			else if (c == '>') 
      			{
				t->token = T_RSHIFT;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_GT;
      			}
      			break;
    		case '&':
      			if ((c = next()) == '&') 
      			{
				t->token = T_LOGAND;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_AMPER;
      			}
      			break;
    		case '|':
      			if ((c = next()) == '|') 
      			{
				t->token = T_LOGOR;
      			} 
      			else 
      			{
				putback(c);
				t->token = T_OR;
     			}
      			break;
    		case '\'':
      			t->intvalue = scanch();
      			t->token = T_INTLIT;
      			if(next() != '\'')
				fatal("Expected '\\'' at end of char literal");
      			break;
    		case '"':
      			scanstr(Text);
      			t -> token = T_STRLIT;
      			break;
    		default:
      			if(isdigit(c)) 
      			{
				t->intvalue = scanint(c);
				t->token = T_INTLIT;
				break;
      			} 
      			else if (isalpha(c) || '_' == c) 
      			{
				scanident(c, Text, TEXTLEN);

				if ((tokentype = keyword(Text)) != 0) 
				{
	  				t->token = tokentype;
	  				break;
				}
				t->token = T_IDENT;
				break;
      			}
      			fatalc("Unrecognised character", c);
  	}

  	t -> tokstr = Tstring[t->token];
  	return (1);
}	
