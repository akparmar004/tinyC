parser: expr.c interp.c main.c scan.c tree.c
	cc -o parser -g expr.c interp.c main.c scan.c tree.c
	
parser2: expr2.c interp.c main.c scan.c tree.c
	cc -o parser2 -g expr2.c interp.c main.c scan.c tree.c
	
test: parser
	-(./parser input; \
	 ./parser input2; \
	 ./parser input3)

test2: parser2
	-(./parser2 input; \
	 ./parser2 input2; \
	 ./parser2 input3)
