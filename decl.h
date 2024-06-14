//function prototypes for all compiler files

// scan.c
void reject_token(struct token *t);
int scan(struct token *t);

// tree.c
ast *mkastnode(int op, int type, ast *left, ast *mid, ast *right, symt *sym, int intvalue);
ast *mkastleaf(int op, int type, symt *sym, int intvalue);
ast *mkastunary(int op, int type, ast *left, symt *sym, int intvalue);
void dumpAST(ast *n, int label, int parentASTop);

// gen.c
int genlabel(void);
int genAST(ast *n, int reg, int parentASTop);
void genpreamble();
void genpostamble();
void genfreeregs();
void genglobsym(symt *node);
int genglobstr(char *strvalue);
int genprimsize(int type);
void genreturn(int reg, int id);

// cg.c
void cgtextseg();
void cgdataseg();
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
void cgfuncpreamble(symt *sym);
void cgfuncpostamble(symt *sym);
int cgloadint(int value, int type);
int cgloadglob(symt *sym, int op);
int cgloadlocal(symt *sym, int op);
int cgloadglobstr(int label);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
int cgshlconst(int r, int val);
int cgcall(symt *sym, int numargs);
void cgcopyarg(int r, int argposn);
int cgstorglob(int r, symt *sym);
int cgstorlocal(int r, symt *sym);
void cgglobsym(symt *node);
void cgglobstr(int l, char *strvalue);
int cgcompare_and_set(int ASTop, int r1, int r2);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label);
void cglabel(int l);
void cgjump(int l);
int cgwiden(int r, int oldtype, int newtype);
int cgprimsize(int type);
void cgreturn(int reg, symt *sym);
int cgaddress(symt *sym);
int cgderef(int r, int type);
int cgstorderef(int r1, int r2, int type);
int cgnegate(int r);
int cginvert(int r);
int cglognot(int r);
int cgboolean(int r, int op, int label);
int cgand(int r1, int r2);
int cgor(int r1, int r2);
int cgxor(int r1, int r2);
int cgshl(int r1, int r2);
int cgshr(int r1, int r2);

// expr.c
ast *binexpr(int ptp);

// stmt.c
ast *compound_statement(void);

// misc.c
void match(int t, char *what);
void semi(void);
void lbrace(void);
void rbrace(void);
void lparen(void);
void rparen(void);
void ident(void);
void fatal(char *s);
void fatals(char *s1, char *s2);
void fatald(char *s, int d);
void fatalc(char *s, int c);

// sym.c
void appendsym(symt **head, symt **tail, symt *node);
symt *newsym(char *name, int type, int stype, int class, int size, int posn);
symt *addglob(char *name, int type, int stype, int class, int size);
symt *addlocl(char *name, int type, int stype, int class, int size);
symt *addparm(char *name, int type, int stype, int class, int size);
symt *findglob(char *s);
symt *findlocl(char *s);
symt *findsymbol(char *s);
symt *findcomposite(char *s);
void clear_symtable(void);
void freeloclsyms(void);

// decl.c
symt *var_declaration(int type, int class);
ast *function_declaration(int type);
void global_declarations(void);

// types.c
int inttype(int type);
int ptrtype(int type);
int parse_type(void);
int pointer_to(int type);
int value_at(int type);
ast *modify_type(ast *tree, int rtype, int op);
