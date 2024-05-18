#include "defs.h"
#include "data.h"
#include "decl.h"

//symbol table functions

static int Globs = 0;		//position of next free global symbol slot

//determine if the symbol s is in the global symbol table. return its slot position or -1 if not found.
int findglob(char *s) 
{
  	int i;

  	for(i = 0; i < Globs; i++) 
	{
    		if(*s == *Gsym[i].name && !strcmp(s, Gsym[i].name))
      			return i;
  	}
  	return -1;
}

//get the position of a new global symbol slot, or die if we've run out of positions.
static int newglob(void) 
{
  	int p;

  	if((p = Globs++) >= NSYMBOLS)
    		fatal("Too many global symbols");
  	return p;
}

//add a global symbol to the symbol table. return the slot number in the symbol table
int addglob(char *name) 
{
  	int y;

  	//if this is already in the symbol table, return the existing slot
  	if((y = findglob(name)) != -1)
    		return (y);

  	//otherwise get a new slot, fill it in and return the slot number
  	y = newglob();
  	Gsym[y].name = strdup(name);
  	return y;
}
