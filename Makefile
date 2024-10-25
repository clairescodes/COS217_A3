# Compiler and flags
CC = gcc217
CFLAGS = 

# Object files for symtablelist and symtablehash
OBJS_LIST = symtablelist.o testsymtable.o
OBJS_HASH = symtablehash.o testsymtable.o

# Default target: Build both executables
all: testsymtablelist testsymtablehash

# Build testsymtablelist executable
testsymtablelist: $(OBJS_LIST)
	$(CC) $(CFLAGS) -o testsymtablelist $(OBJS_LIST)

# Build testsymtablehash executable
testsymtablehash: $(OBJS_HASH)
	$(CC) $(CFLAGS) -o testsymtablehash $(OBJS_HASH)

# Compile symtablelist.o
symtablelist.o: symtablelist.c symtable.h
	$(CC) $(CFLAGS) -c symtablelist.c

# Compile symtablehash.o
symtablehash.o: symtablehash.c symtable.h
	$(CC) $(CFLAGS) -c symtablehash.c

# Compile testsymtable.o
testsymtable.o: testsymtable.c symtable.h
	$(CC) $(CFLAGS) -c testsymtable.c

# delete all object files and executable binary files 
clean:
	rm -f *.o testsymtablelist testsymtablehash
