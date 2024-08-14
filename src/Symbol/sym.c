#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//symbol table functions


void appendsym(symt **head, symt **tail, symt *node) 
{
  	if(head == NULL || tail == NULL || node == NULL)
    		fatal("Either head, tail or node is NULL in appendsym");

  	if(*tail) 
  	{
    		(*tail)->next = node;
    		*tail = node;
  	} 
  	else
    		*head = *tail = node;
  	
  	node->next = NULL;
}

symt *newsym(char *name, int type, symt *ctype, int stype, int class, int nelems, int posn) 
{

  	symt *node = (symt *) malloc(sizeof(symt));
  	
  	if(node == NULL)
    		fatal("Unable to malloc a symbol table node in newsym");

  	if(name == NULL)
    		node->name = NULL;
  	else
    		node->name = strdup(name);
  	
  	node->type = type;
  	node->ctype = ctype;
  	node->stype = stype;
  	node->class = class;
  	node->nelems = nelems;

  	if(ptrtype(type) || inttype(type))
    		node->size = nelems * typesize(type, ctype);

  	node->st_posn = posn;
  	node->next = NULL;
  	node->member = NULL;
  	node->initlist = NULL;
  	
  	return node;
}

symt *addglob(char *name, int type, symt *ctype, int stype, int class, int nelems, int posn) 
{
  	symt *sym = newsym(name, type, ctype, stype, class, nelems, posn);
  	if(type == P_STRUCT || type == P_UNION)
    		sym->size = ctype->size;
  	
  	appendsym(&Globhead, &Globtail, sym);
  	
  	return sym;
}

symt *addlocl(char *name, int type, symt *ctype, int stype, int nelems) 
{
  	symt *sym = newsym(name, type, ctype, stype, C_LOCAL, nelems, 0);
  	
  	if(type == P_STRUCT || type == P_UNION)
    		sym->size = ctype->size;
  	
  	appendsym(&Loclhead, &Locltail, sym);
  	return sym;
}

symt *addparm(char *name, int type, symt *ctype, int stype) 
{
  	symt *sym = newsym(name, type, ctype, stype, C_PARAM, 1, 0);
  	appendsym(&Parmhead, &Parmtail, sym);
  	
  	return sym;
}

symt *addmemb(char *name, int type, symt *ctype, int stype, int nelems) 
{
  	symt *sym = newsym(name, type, ctype, stype, C_MEMBER, nelems, 0);
  	
  	if(type == P_STRUCT || type == P_UNION)
   		sym->size = ctype->size;
  	
  	appendsym(&Membhead, &Membtail, sym);
  	
  	return sym;
}

symt *addstruct(char *name) 
{
  	symt *sym = newsym(name, P_STRUCT, NULL, 0, C_STRUCT, 0, 0);
  	appendsym(&Structhead, &Structtail, sym);
  	
  	return sym;
}

symt *addunion(char *name) 
{
  	symt *sym = newsym(name, P_UNION, NULL, 0, C_UNION, 0, 0);
  	appendsym(&Unionhead, &Uniontail, sym);
  	
  	return sym;
}

symt *addenum(char *name, int class, int value) 
{
  	symt *sym = newsym(name, P_INT, NULL, 0, class, 0, value);
  	appendsym(&Enumhead, &Enumtail, sym);
  	
  	return sym;
}

symt *addtypedef(char *name, int type, symt *ctype) 
{
  	symt *sym = newsym(name, type, ctype, 0, C_TYPEDEF, 0, 0);
  	appendsym(&Typehead, &Typetail, sym);
  	
  	return sym;
}

static symt *findsyminlist(char *s, symt *list, int class) 
{
  	for(; list != NULL; list = list->next)
    		if((list->name != NULL) && !strcmp(s, list->name))
      			if(class == 0 || class == list->class)
				return list;
  		
  	return NULL;
}

symt *findglob(char *s) 
{
  	return findsyminlist(s, Globhead, 0);
}

symt *findlocl(char *s) 
{
  	symt *node;

  	if(Functionid) 
  	{
    		node = findsyminlist(s, Functionid->member, 0);
    		if(node)
      			return node;
  	}
  	return findsyminlist(s, Loclhead, 0);
}

symt *findsymbol(char *s) 
{
  	symt *node;

  	if(Functionid) 
  	{
    		node = findsyminlist(s, Functionid->member, 0);
    		if(node)
      			return node;
  	}
  	node = findsyminlist(s, Loclhead, 0);
  	if(node)
    		return node;
  	
  	return findsyminlist(s, Globhead, 0);
}

symt *findmember(char *s)
{
  	return findsyminlist(s, Membhead, 0);
}

symt *findstruct(char *s) 
{
  	return findsyminlist(s, Structhead, 0);
}

symt *findunion(char *s) 
{
  	return findsyminlist(s, Unionhead, 0);
}

symt *findenumtype(char *s) 
{
  	return (findsyminlist(s, Enumhead, C_ENUMTYPE));
}

symt *findenumval(char *s) 
{
  	return (findsyminlist(s, Enumhead, C_ENUMVAL));
}

symt *findtypedef(char *s) 
{
  	return (findsyminlist(s, Typehead, 0));
}

void clear_symtable(void) 
{
  	Globhead = Globtail = NULL;
  	Loclhead = Locltail = NULL;
  	Parmhead = Parmtail = NULL;
  	Membhead = Membtail = NULL;
  	Structhead = Structtail = NULL;
  	Unionhead = Uniontail = NULL;
  	Enumhead = Enumtail = NULL;
  	Typehead = Typetail = NULL;
}

void freeloclsyms(void) 
{
  	Loclhead = Locltail = NULL;
  	Parmhead = Parmtail = NULL;
  	Functionid = NULL;
}

void freestaticsyms(void) 
{
  	symt *g, *prev = NULL;

  	for(g = Globhead; g != NULL; g = g->next) 
  	{
    		if(g->class == C_STATIC) 
    		{

      			if(prev != NULL)
				prev->next = g->next;
      			else
				Globhead->next = g->next;

      			if(g == Globtail) 
      			{
				if(prev != NULL)
	  				Globtail = prev;
				else
	  				Globtail = Globhead;
      			}
    		}
  	}

  	prev = g;
}

static void dumpsym(symt *sym, int indent) 
{
  	int i;

  	for(i = 0; i < indent; i++)
    		printf(" ");
  	
  	switch(sym->type & (~0xf)) 
  	{
    		case P_VOID:
      			printf("void ");
      			break;
    		case P_CHAR:
      			printf("char ");
      			break;
    		case P_INT:
      			printf("int ");
      			break;
    		case P_LONG:
      			printf("long ");
      			break;
    		case P_STRUCT:
      			if (sym->ctype != NULL)
				printf("struct %s ", sym->ctype->name);
      			else
				printf("struct %s ", sym->name);
      			break;
    		case P_UNION:
      			if (sym->ctype != NULL)
				printf("union %s ", sym->ctype->name);
      			else
				printf("union %s ", sym->name);
      			break;
    		default:
      			printf("unknown type ");
  	}

  	for(i = 0; i < (sym->type & 0xf); i++)
    		printf("*");
  	
  	printf("%s", sym->name);

  	switch(sym->stype) 
  	{
    		case S_VARIABLE:
      			break;
    		case S_FUNCTION:
      			printf("()");
      			break;
    		case S_ARRAY:
      			printf("[]");
     			break;
    		default:
      			printf(" unknown stype");
  	}

  	switch(sym->class) 
  	{
    		case C_GLOBAL:
      			printf(": global");
      			break;
    		case C_LOCAL:
      			printf(": local");
      			break;
    		case C_PARAM:
      			printf(": param");
      			break;
    		case C_EXTERN:
      			printf(": extern");
      			break;
    		case C_STATIC:
      			printf(": static");
      			break;
    		case C_STRUCT:
      			printf(": struct");
      			break;
    		case C_UNION:
      			printf(": union");
      			break;
    		case C_MEMBER:
      			printf(": member");
      			break;
    		case C_ENUMTYPE:
      			printf(": enumtype");
      			break;
    		case C_ENUMVAL:
      			printf(": enumval");
      			break;
    		case C_TYPEDEF:
      			printf(": typedef");
      			break;
    		default:
      			printf(": unknown class");
  	}

  	switch(sym->stype) 
  	{
    		case S_VARIABLE:
      			if(sym->class == C_ENUMVAL)
				printf(", value %d\n", sym->st_posn);
      			else
				printf(", size %d\n", sym->size);
      			break;
    		case S_FUNCTION:
      			printf(", %d params\n", sym->nelems);
      			break;
    		case S_ARRAY:
      			printf(", %d elems, size %d\n", sym->nelems, sym->size);
      			break;
  	}

  	switch(sym->type & (~0xf)) 
  	{
    		case P_STRUCT:
    		case P_UNION:
      			dumptable(sym->member, NULL, 4);
  	}

  	switch(sym->stype) 
  	{
    		case S_FUNCTION:
      			dumptable(sym->member, NULL, 4);
  	}
}

void dumptable(symt *head, char *name, int indent) 
{
  	symt *sym;

  	if(head != NULL && name != NULL)
    		printf("%s\n--------\n", name);
  	
  	for(sym = head; sym != NULL; sym = sym->next)
    		dumpsym(sym, indent);
}

void dumpsymtables(void) 
{
  	dumptable(Globhead, "Global", 0);
  	printf("\n");
  	dumptable(Enumhead, "Enums", 0);
  	printf("\n");
  	dumptable(Typehead, "Typedefs", 0);
}
