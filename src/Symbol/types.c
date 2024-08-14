#include "../../include/defs.h"
#include "../../include/data.h"
#include "../../include/decl.h"

//types and type handling

int inttype(int type) 
{
  	return (((type & 0xf) == 0) && (type >= P_CHAR && type <= P_LONG));
}

int ptrtype(int type) 
{
  	return ((type & 0xf) != 0);
}

int pointer_to(int type) 
{
  	if ((type & 0xf) == 0xf)
    		fatald("Unrecognised in pointer_to: type", type);
  	
  	return (type + 1);
}

int value_at(int type) 
{
  	if ((type & 0xf) == 0x0)
    		fatald("Unrecognised in value_at: type", type);
  	
  	return (type - 1);
}

int typesize(int type, symt *ctype)
{
  	if (type == P_STRUCT || type == P_UNION)
    		return (ctype->size);
  	
  	return genprimsize(type);
}

ast *modify_type(ast *tree, int rtype, symt *rctype, int op) 
{
  	int ltype;
  	int lsize, rsize;

  	ltype = tree->type;

  	if(op==A_LOGOR || op==A_LOGAND) 
  	{
    		if (!inttype(ltype) && !ptrtype(ltype))
      			return NULL;
    		
    		if (!inttype(ltype) && !ptrtype(rtype))
      			return NULL;
    		
    		return tree;
  	}

  	if(ltype == P_STRUCT || ltype == P_UNION)
    		fatal("Don't know how to do this yet");
  	if(rtype == P_STRUCT || rtype == P_UNION)
    		fatal("Don't know how to do this yet");

  	if(inttype(ltype) && inttype(rtype)) 
  	{

    		if(ltype == rtype)
      			return tree;

    		lsize = typesize(ltype, NULL);
    		rsize = typesize(rtype, NULL);

    		if(lsize > rsize)
      			return NULL;

    		if(rsize > lsize)
      			return (mkastunary(A_WIDEN, rtype, NULL, tree, NULL, 0));
  	}
  	if(ptrtype(ltype) && ptrtype(rtype)) 
  	{
    		if(op >= A_EQ && op <= A_GE)
      			return tree;

    		if(op == 0 && (ltype == rtype || ltype == pointer_to(P_VOID)))
      			return tree;
  	}
  	if(op == A_ADD || op == A_SUBTRACT || op == A_ASPLUS || op == A_ASMINUS) 
  	{

    		if (inttype(ltype) && ptrtype(rtype)) 
    		{
      			rsize = genprimsize(value_at(rtype));
      			if (rsize > 1)
				return mkastunary(A_SCALE, rtype, rctype, tree, NULL, rsize);
      			else
				return tree;		// Size 1, no need to scale
    		}
  	}
  	return NULL;
}
