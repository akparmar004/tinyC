headfiles = decl.h defs.h data.h 

files = cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c

filesn = cgn.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c

jarvis: $(files) $(headfiles)
	cc -o jarvis -g -Wall $(files)

jarvisn: $(filesn) $(headfiles)
	cc -D__NASM__ -o jarvisn -g -Wall $(filesn)

test: jarvis tests/input55.c 
	./jarvis 
	cc -o out out.s 
	./out

test28n: jarvisn tests/input28.c lib/printint.c
	./jarvisn tests/input28.c
	nasm -f elf64 out.s
	cc -no-pie -o out lib/printint.c out.o
	./out
