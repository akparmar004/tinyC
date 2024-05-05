SRCS= cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c
SRCN= cgn.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c

comp1: $(SRCS)
	cc -o comp1 -g $(SRCS)

compn: $(SRCN)
	cc -o compn -g $(SRCN)


test: comp1 input input2
	./comp1 input
	cc -o out out.s
	./out
	./comp1 input2
	cc -o out out.s
	./out

testn: compn input input2
	./compn input
	nasm -f elf64 out.s
	cc -no-pie -o out out.o
	./out
	./compn input2
	nasm -f elf64 out.s
	cc -no-pie -o out out.o
	./out
