//function prototypes for all files functions..
//scan.c
int scan(struct token *t);

//tree.c
ast *mkastnode(int op, ast *left, ast *right, int intvalue);
ast *mkastleaf(int op, int intvalue);
ast *mkastunary(int op, ast *left, int intvalue);

//expr.c
ast *binexpr(int ptp);

//gen.c
int genAST(ast *n);
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);


//cg.c
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgload(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);

//stmt.c
void statements(void);

//misc.c
void match(int t, char *what);
void semi(void);
