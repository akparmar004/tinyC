SRCS= cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c
SRCN= cgn.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c

comp1: $(SRCS)
	cc -o comp1 -g $(SRCS)

compn: $(SRCN)
	cc -o compn -g $(SRCN)

test: comp1 input4
	./comp1 input4
	cc -o out out.s
	./out

testn: compn input4
	./compn input4
	nasm -f elf64 out.s
	cc -no-pie -o out out.o
	./out
