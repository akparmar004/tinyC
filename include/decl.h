//function prototypes
//scan.c
void reject_token(struct token *t);
int scan(struct token *t);

//tree.c
ast *mkastnode(int op, int type, symt *ctype, ast *left, ast *mid, ast *right, symt *sym, int intvalue);
ast *mkastleaf(int op, int type, symt *ctype, symt *sym, int intvalue);
ast *mkastunary(int op, int type, symt *ctype, ast *left, symt *sym, int intvalue);
void dumpAST(ast *n, int label, int level);

// gen.c
int genlabel(void);
int genAST(ast *n, int iflabel, int looptoplabel, int loopendlabel, int parentASTop);
void genpreamble(char *filename);
void genpostamble();
void genfreeregs(int keepreg);
void genglobsym(symt *node);
int genglobstr(char *strvalue, int append);
void genglobstrend(void);
int genprimsize(int type);
int genalign(int type, int offset, int direction);
void genreturn(int reg, int id);

// cg.c
int cgprimsize(int type);
int cgalign(int type, int offset, int direction);
void cgtextseg();
void cgdataseg();
int cgalloctemp(void);
void cgfreeallregs(int keepreg);
void cgfreereg(int reg);
void cgspillregs(void);
void cgpreamble(char *filename);
void cgpostamble();
void cgfuncpreamble(symt *sym);
void cgfuncpostamble(symt *sym);
int cgloadint(int value, int type);
int cgloadvar(symt *sym, int op);
int cgloadglobstr(int label);
int cgadd(int r1, int r2, int type);
int cgsub(int r1, int r2, int type);
int cgmul(int r1, int r2, int type);
int cgdivmod(int r1, int r2, int op, int type);
int cgshlconst(int r, int val, int type);
int cgcall(symt *sym, int numargs, int *arglist, int *typelist);
void cgcopyarg(int r, int argposn);
int cgstorglob(int r, symt *sym);
int cgstorlocal(int r, symt *sym);
void cgglobsym(symt *node);
void cgglobstr(int l, char *strvalue, int append);
void cgglobstrend(void);
int cgcompare_and_set(int ASTop, int r1, int r2, int type);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label, int type);
void cglabel(int l);
void cgjump(int l);
int cgwiden(int r, int oldtype, int newtype);
void cgreturn(int reg, symt *sym);
int cgaddress(symt *sym);
int cgderef(int r, int type);
int cgstorderef(int r1, int r2, int type);
int cgnegate(int r, int type);
int cginvert(int r, int type);
int cglognot(int r, int type);
void cgloadboolean(int r, int val, int type);
int cgboolean(int r, int op, int label, int type);
int cgand(int r1, int r2, int type);
int cgor(int r1, int r2, int type);
int cgxor(int r1, int r2, int type);
int cgshl(int r1, int r2, int type);
int cgshr(int r1, int r2, int type);
void cgswitch(int reg, int casecount, int toplabel, int *caselabel, int *caseval, int defaultlabel);
void cgmove(int r1, int r2, int type);
void cglinenum(int line);
int cgcast(int t, int oldtype, int newtype);

// expr.c
ast *expression_list(int endtoken);
ast *binexpr(int ptp);

// stmt.c
ast *compound_statement(int inswitch);

// misc.c
void match(int t, char *what);
void semi(void);
void lbrace(void);
void rbrace(void);
void lparen(void);
void rparen(void);
void ident(void);
void comma(void);
void fatal(char *s);
void fatals(char *s1, char *s2);
void fatald(char *s, int d);
void fatalc(char *s, int c);

// sym.c
void appendsym(symt **head, symt **tail, symt *node);
symt *newsym(char *name, int type, symt *ctype, int stype, int class, int nelems, int posn);
symt *addglob(char *name, int type, symt *ctype, int stype, int class, int nelems, int posn);
symt *addlocl(char *name, int type, symt *ctype, int stype, int nelems);
symt *addparm(char *name, int type, symt *ctype, int stype);
symt *addstruct(char *name);
symt *addunion(char *name);
symt *addmemb(char *name, int type, symt *ctype, int stype, int nelems);
symt *addenum(char *name, int class, int value);
symt *addtypedef(char *name, int type, symt *ctype);
symt *findglob(char *s);
symt *findlocl(char *s);
symt *findsymbol(char *s);
symt *findmember(char *s);
symt *findstruct(char *s);
symt *findunion(char *s);
symt *findenumtype(char *s);
symt *findenumval(char *s);
symt *findtypedef(char *s);
void clear_symtable(void);
void freeloclsyms(void);
void freestaticsyms(void);
void dumptable(symt *head, char *name, int indent);
void dumpsymtables(void);

// decl.c
int parse_type(symt **ctype, int *class);
int parse_stars(int type);
int parse_cast(symt **ctype);
int declaration_list(symt **ctype, int class, int et1, int et2, ast **gluetree);
void global_declarations(void);

// types.c
int inttype(int type);
int ptrtype(int type);
int pointer_to(int type);
int value_at(int type);
int typesize(int type, symt *ctype);
ast *modify_type(ast *tree, int rtype, symt *rctype, int op);

// opt.c
ast *optimise(ast *n);
