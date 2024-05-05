comp1: cg.c expr.c gen.c main.c misc.c scan.c stmt.c tree.c
	cc -o comp1 -g cg.c expr.c gen.c main.c misc.c scan.c stmt.c tree.c

compn: cgn.c expr.c gen.c main.c misc.c scan.c stmt.c tree.c
	cc -o compn -g cgn.c expr.c gen.c main.c misc.c scan.c stmt.c tree.c

test: comp1 input
	./comp1 input
	cc -o out out.s
	./out

testn: compn input
	./compn input
	nasm -f elf64 out.s
	cc -no-pie -o out out.o
	./out
