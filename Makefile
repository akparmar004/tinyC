#define the location of the include directory and location to install the compiler binary
INCDIR=/tmp/include
BINDIR=/tmp

HSRCS= include/data.h include/decl.h include/defs.h include/incdir.h
SRCS= src/Code_generator/cg.c src/Parser/decl.c src/Parser/expr.c \
      src/Code_generator/gen.c src/Main/main.c src/Lexical_scanner/misc.c \
      src/Tree/opt.c src/Lexical_scanner/scan.c src/Parser/stmt.c \
      src/Symbol/sym.c src/Tree/tree.c src/Symbol/types.c	 

ASRCS= src/Code_generator/arm.c src/Parser/decl.c src/Parser/expr.c \
      src/Code_generator/gen.c src/Main/main.c src/Lexical_scanner/misc.c \
      src/Tree/opt.c src/Lexical_scanner/scan.c src/Parser/stmt.c \
      src/Symbol/sym.c src/Tree/tree.c src/Symbol/types.c

tinyC:    $(SRCS) $(HSRCS)
	echo "#define INCDIR \"$(INCDIR)\"" > include/incdir.h
	cc -o bin/tinyC -g -Wall -DINCDIR=\"$(INCDIR)\" $(SRCS)
	
tinyCarm: $(ASRCS) $(HSRCS)
	echo "#define INCDIR \"$(INCDIR)\"" > include/incdir.h
	cc -o bin/tinyCarm -g -Wall $(ASRCS) 
	
incdir.h:
	echo "#define INCDIR \"$(INCDIR)\"" > include/incdir.h

install: bin/tinyC
	mkdir -p $(INCDIR)
	rsync -a include/. $(INCDIR)
	cp bin/tinyC $(BINDIR)
	chmod +x $(BINDIR)/tinyC
	
installarm: bin/tinyCarm
	mkdir -p $(INCDIR)
	rsync -a include/. $(INCDIR)
	cp bin/tinyCarm $(BINDIR)
	chmod +x $(BINDIR)/tinyCarm
