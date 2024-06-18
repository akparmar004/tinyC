#include "defs.h"
#include "data.h"
#include "decl.h"

//symbol table functions to store the info about idents...

//append a node to the singly-linked list pointed to by head or tail
void appendsym(symt **head, symt **tail, symt *node)
{

  	//check for valid pointers
  	if(head == NULL || tail == NULL || node == NULL)
    		fatal("Either head, tail or node is NULL in appendsym");

  	//append to the list
  	if(*tail) 
  	{
    		(*tail) -> next = node;
    		*tail = node;
  	}
  	else
    		*head = *tail = node;
  	
  	node -> next = NULL;
}

symt *newsym(char *name, int type, int stype, int class, int size, int posn) 
{

  	//get new node
  	symt *node = (symt *) malloc(sizeof(symt));
  	if(node == NULL)
    		fatal("Unable to malloc a symbol table node in newsym");

  	// Fill in the values
  	node -> name = strdup(name);
	node -> type = type;
  	node->stype = stype;
  	node->class = class;
  	node->size = size;
  	node->posn = posn;
  	node->next = NULL;
  	node->member = NULL;

  	//generate any global space
  	if(class == C_GLOBAL)
    		genglobsym(node);
  	
  	return node;
}

//add symbol to global symbol list
symt *addglob(char *name, int type, int stype, int class, int size) 
{
  	symt *sym = newsym(name, type, stype, class, size, 0);
	appendsym(&Globhead, &Globtail, sym);
  	
  	return sym;
}

//add a symbol to the local symbol list
symt *addlocl(char *name, int type, int stype, int class, int size) 
{
  	symt *sym = newsym(name, type, stype, class, size, 0);
  	appendsym(&Loclhead, &Locltail, sym);
  	
  	return sym;
}

// Add a symbol to the parameter list
symt *addparm(char *name, int type, int stype, int class, int size) 
{
 	symt *sym = newsym(name, type, stype, class, size, 0);
  	appendsym(&Parmhead, &Parmtail, sym);
  	
  	return sym;
}

//search for a symbol in a specific list, return a pointer to the found node or NULL if not found.
static symt *findsyminlist(char *s, symt *list) 
{
  	for( ; list != NULL; list = list -> next)
    		if((list->name != NULL) && !strcmp(s, list->name))
      			return list;
  	
  	return NULL;
}

//determine if the symbol s is in the global symbol table. return its slot position or -1 if not found.
symt *findglob(char *s) 
{
  	return findsyminlist(s, Globhead);
}

//determine if the symbol s is in the local symbol table. 
//return a pointer to the found node or NULL if not found.
symt *findlocl(char *s) 
{
  	symt *node;

  	//look for a parameter if we are in a function's body
  	if(Functionid) 
	{
    		node = findsyminlist(s, Functionid->member);
    		if(node)
      			return node;
  	}
  	return findsyminlist(s, Loclhead);
}

// Determine if the symbol s is in the symbol table.
// Return a pointer to the found node or NULL if not found.
symt *findsymbol(char *s) 
{
  	symt *node;

  	// Look for a parameter if we are in a function's body
  	if(Functionid) 
  	{
    		node = findsyminlist(s, Functionid->member);
    		if(node)
      			return node;
  	}

  	// Otherwise, try the local and global symbol lists
  	node = findsyminlist(s, Loclhead);
  	if(node)
    		return node;
  	
  	return findsyminlist(s, Globhead);
}

//find a composite type, return a pointer to the found node or NULL if not found.
symt *findcomposite(char *s) 
{
  	return findsyminlist(s, Comphead);
}

//reset the contents of the symbol table
void clear_symtable(void) 
{
  	Globhead = Globtail = NULL;
  	Loclhead = Locltail = NULL;
  	Parmhead = Parmtail = NULL;
  	Comphead = Comptail = NULL;
}

//clear all the entries in the local symbol table
void freeloclsyms(void) 
{
  	Loclhead = Locltail = NULL;
  	Parmhead = Parmtail = NULL;
  	Functionid = NULL;
}
