#define the location of the include directory and location to install the compiler binary
INCDIR=/tmp/include
BINDIR=/tmp

HSRCS= data.h decl.h defs.h incdir.h
SRCS= cg.c decl.c expr.c gen.c main.c misc.c opt.c scan.c stmt.c sym.c tree.c types.c

cwj: $(SRCS) $(HSRCS)
	cc -o cwj -g -Wall -DINCDIR=\"$(INCDIR)\" $(SRCS)

incdir.h:
	echo "#define INCDIR \"$(INCDIR)\"" > incdir.h

install: cwj
	mkdir -p $(INCDIR)
	rsync -a include/. $(INCDIR)
	cp cwj $(BINDIR)
	chmod +x $(BINDIR)/cwj
