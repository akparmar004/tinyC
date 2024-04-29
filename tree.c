struct ASTnode *mkastnode(int op, struct ASTnode *left, struct ASTnode *right, int intvalue)
{
	struct ASTnode *n;

	n = (struct ASTnode *)malloc(sizeof(struct ASTnode));
	if(n == NULL)
	{
		fprintf(stderr,"Unable to malloc in mkastnode()\n");
		exit(1);
	}
	//copy the data and return it
	n -> op = op;
	n -> left = left;
	n -> right = right;
	n -> intvalue = intvalue;

	return (n);
}

