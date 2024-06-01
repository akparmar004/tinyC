files = cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c

filesn = cgn.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c

jarvis: $(files)
	cc -o jarvis -g -Wall $(files)

jarvisn: $(filesn)
	cc -o jarvisn -g -Wall $(filesn)

test: jarvis tests/runtests
	(cd tests; chmod +x runtests; ./runtests)

testn: jarvisn tests/runtestsn
	(cd tests; chmod +x runtestsn; ./runtestsn)

test20: jarvis tests/input20.c lib/printint.c
	./jarvis tests/input20.c
	cc -o out out.s lib/printint.c
	./out

test20n: jarvisn tests/input20.c lib/printint.c
	./jarvisn tests/input20.c
	nasm -f elf64 out.s
	cc -no-pie -o out lib/printint.c out.o
	./out
