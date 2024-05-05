
// Function prototypes for all compiler files
// Copyright (c) 2019 Warren Toomey, GPL3
// scan.c
int scan(struct token *t);

// tree.c
ast *mkastnode(int op, ast *left, ast *right, int intvalue);
ast *mkastleaf(int op, int intvalue);
ast *mkastunary(int op, ast *left, int intvalue);

// gen.c
int genAST(ast *n, int reg);
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(char *s);

// cg.c
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgloadint(int value);
int cgloadglob(char *identifier);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);
int cgstorglob(int r, char *identifier);
void cgglobsym(char *sym);

// expr.c
ast *binexpr(int ptp);

// stmt.c
void statements(void);

// misc.c
void match(int t, char *what);
void semi(void);
void ident(void);
void fatal(char *s);
void fatals(char *s1, char *s2);
void fatald(char *s, int d);
void fatalc(char *s, int c);

// sym.c
int findglob(char *s);
int addglob(char *name);

// decl.c
void var_declaration(void);
