//function prototypes for all compiler functions..

int scan(struct token *t);
ast *mkastnode(int op, ast *left, ast *right, int intvalue);
ast *mkastleaf(int op, int intvalue);
ast *mkastunary(int op, ast *left, int intvalue);
ast *binexpr(void);
int interpretAST(ast *n);
