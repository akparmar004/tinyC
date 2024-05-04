//function prototypes for all files functions..

int scan(struct token *t);
ast *mkastnode(int op, ast *left, ast *right, int intvalue);
ast *mkastleaf(int op, int intvalue);
ast *mkastunary(int op, ast *left, int intvalue);
ast *binexpr(int rbp);

int interpretAST(ast *n);
void generatecode(ast* n);

void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgload(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);
