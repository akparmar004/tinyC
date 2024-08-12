#include "defs.h"
#include "data.h"
#include "decl.h"

//symbol table functions


//append node to singly-linked list pointed to by head or tail
void appendsym(symt **head, symt **tail, symt *node) 
{
  	//check for valid pointers
  	if(head == NULL || tail == NULL || node == NULL)
    		fatal("Either head, tail or node is NULL in appendsym");

  	//append to the list
  	if(*tail) 
  	{
    		(*tail)->next = node;
    		*tail = node;
  	} 
  	else
    		*head = *tail = node;
  	
  	node->next = NULL;
}

//create symbol node to be added to symbol table list.
//set up node's: 
// + type: char, int etc.
// + ctype: composite type pointer for struct/union
// + structural type: var, function, array etc.
// + size: number of elements, or endlabel: end label for a function
// + posn: Position information for local symbols
// Return a pointer to the new node
symt *newsym(char *name, int type, symt *ctype, int stype, int class, int nelems, int posn) 
{

  	// Get a new node
  	symt *node = (symt *) malloc(sizeof(symt));
  	
  	if(node == NULL)
    		fatal("Unable to malloc a symbol table node in newsym");

  	// Fill in the values
  	if(name == NULL)
    		node->name = NULL;
  	else
    		node->name = strdup(name);
  	
  	node->type = type;
  	node->ctype = ctype;
  	node->stype = stype;
  	node->class = class;
  	node->nelems = nelems;

  	// For pointers and integer types, set the size of the symbol. structs and union declarations
  	// manually set this up themselves.
  	if(ptrtype(type) || inttype(type))
    		node->size = nelems * typesize(type, ctype);

  	node->st_posn = posn;
  	node->next = NULL;
  	node->member = NULL;
  	node->initlist = NULL;
  	
  	return node;
}

// Add a symbol to the global symbol list
symt *addglob(char *name, int type, symt *ctype, int stype, int class, int nelems, int posn) 
{
  	symt *sym = newsym(name, type, ctype, stype, class, nelems, posn);
  	// For structs and unions, copy the size from the type node
  	if(type == P_STRUCT || type == P_UNION)
    		sym->size = ctype->size;
  	
  	appendsym(&Globhead, &Globtail, sym);
  	
  	return sym;
}

// Add a symbol to the local symbol list
symt *addlocl(char *name, int type, symt *ctype, int stype, int nelems) 
{
  	symt *sym = newsym(name, type, ctype, stype, C_LOCAL, nelems, 0);
  	
  	// For structs and unions, copy the size from the type node
  	if(type == P_STRUCT || type == P_UNION)
    		sym->size = ctype->size;
  	
  	appendsym(&Loclhead, &Locltail, sym);
  	return sym;
}

// Add a symbol to the parameter list
symt *addparm(char *name, int type, symt *ctype, int stype) 
{
  	symt *sym = newsym(name, type, ctype, stype, C_PARAM, 1, 0);
  	appendsym(&Parmhead, &Parmtail, sym);
  	
  	return sym;
}

// Add a symbol to the temporary member list
symt *addmemb(char *name, int type, symt *ctype, int stype, int nelems) 
{
  	symt *sym = newsym(name, type, ctype, stype, C_MEMBER, nelems, 0);
  	
  	// For structs and unions, copy the size from the type node
  	if(type == P_STRUCT || type == P_UNION)
   		sym->size = ctype->size;
  	
  	appendsym(&Membhead, &Membtail, sym);
  	
  	return sym;
}

// Add a struct to the struct list
symt *addstruct(char *name) 
{
  	symt *sym = newsym(name, P_STRUCT, NULL, 0, C_STRUCT, 0, 0);
  	appendsym(&Structhead, &Structtail, sym);
  	
  	return sym;
}

// Add a struct to the union list
symt *addunion(char *name) 
{
  	symt *sym = newsym(name, P_UNION, NULL, 0, C_UNION, 0, 0);
  	appendsym(&Unionhead, &Uniontail, sym);
  	
  	return sym;
}

//add an enum type or value to enum list, class is C_ENUMTYPE or C_ENUMVAL.
//use posn to store int value.
symt *addenum(char *name, int class, int value) 
{
  	symt *sym = newsym(name, P_INT, NULL, 0, class, 0, value);
  	appendsym(&Enumhead, &Enumtail, sym);
  	
  	return sym;
}

//add typedef to typedef list
symt *addtypedef(char *name, int type, symt *ctype) 
{
  	symt *sym = newsym(name, type, ctype, 0, C_TYPEDEF, 0, 0);
  	appendsym(&Typehead, &Typetail, sym);
  	
  	return sym;
}

//search for symbol in specific list, return pointer to found node or NULL if not found.
//if class is not zero, also match on the given class
static symt *findsyminlist(char *s, symt *list, int class) 
{
  	for(; list != NULL; list = list->next)
    		if((list->name != NULL) && !strcmp(s, list->name))
      			if(class == 0 || class == list->class)
				return list;
  		
  	return NULL;
}

// Determine if symbol s is in global symbol table, Return pointer to found node or NULL if not found.
symt *findglob(char *s) 
{
  	return findsyminlist(s, Globhead, 0);
}

// Determine if the symbol s is in local symbol table, Return pointer to the found node or NULL if not found.
symt *findlocl(char *s) 
{
  	symt *node;

  	// Look for a parameter if we are in a function's body
  	if(Functionid) 
  	{
    		node = findsyminlist(s, Functionid->member, 0);
    		if(node)
      			return node;
  	}
  	return findsyminlist(s, Loclhead, 0);
}

// Determine if the symbol s is in the symbol table, Return pointer to the found node or NULL if not found.
symt *findsymbol(char *s) 
{
  	symt *node;

  	// Look for a parameter if we are in a function's body
  	if(Functionid) 
  	{
    		node = findsyminlist(s, Functionid->member, 0);
    		if(node)
      			return node;
  	}
  	// Otherwise, try the local and global symbol lists
  	node = findsyminlist(s, Loclhead, 0);
  	if(node)
    		return node;
  	
  	return findsyminlist(s, Globhead, 0);
}

// Find a member in the member list Return a pointer to the found node or NULL if not found.
symt *findmember(char *s)
{
  	return findsyminlist(s, Membhead, 0);
}

// Find a struct in the struct list Return a pointer to the found node or NULL if not found.
symt *findstruct(char *s) 
{
  	return findsyminlist(s, Structhead, 0);
}

// Find a struct in the union list Return a pointer to the found node or NULL if not found.
symt *findunion(char *s) 
{
  	return findsyminlist(s, Unionhead, 0);
}

// Find an enum type in the enum list Return a pointer to the found node or NULL if not found.
symt *findenumtype(char *s) 
{
  	return (findsyminlist(s, Enumhead, C_ENUMTYPE));
}

// Find an enum value in the enum list Return a pointer to the found node or NULL if not found.
symt *findenumval(char *s) 
{
  	return (findsyminlist(s, Enumhead, C_ENUMVAL));
}

//find type in the tyedef list Return a pointer to the found node or NULL if not found.
symt *findtypedef(char *s) 
{
  	return (findsyminlist(s, Typehead, 0));
}

//reset contents of the symbol table
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

// Clear all the entries in the local symbol table
void freeloclsyms(void) 
{
  	Loclhead = Locltail = NULL;
  	Parmhead = Parmtail = NULL;
  	Functionid = NULL;
}

// Remove all static symbols from the global symbol table
void freestaticsyms(void) 
{
  	// g points at current node, prev at the previous one
  	symt *g, *prev = NULL;

  	// Walk the global table looking for static entries
  	for(g = Globhead; g != NULL; g = g->next) 
  	{
    		if(g->class == C_STATIC) 
    		{

      			// If there's a previous node, rearrange the prev pointer
      			// to skip over the current node. If not, g is the head, so do the same to Globhead
      			if(prev != NULL)
				prev->next = g->next;
      			else
				Globhead->next = g->next;

      			// If g is the tail, point Globtail at the previous node (if there is one), or Globhead
      			if(g == Globtail) 
      			{
				if(prev != NULL)
	  				Globtail = prev;
				else
	  				Globtail = Globhead;
      			}
    		}
  	}

  	// Point prev at g before we move up to the next node
  	prev = g;
}

// Dump a single symbol
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

// Dump one symbol table
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
