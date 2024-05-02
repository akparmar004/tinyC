//function prototypes for all files functions..

int scan(struct token *t);
ast *mkastnode(int op, ast *left, ast *right, int intvalue);
ast *mkastleaf(int op, int intvalue);
ast *mkastunary(int op, ast *left, int intvalue);
ast *binexpr(int rbp);
int interpretAST(ast *n);
