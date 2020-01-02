bminor: symbol.o scope.o bminor.o parser.o scanner.o decl.o stmt.o expr.o param_list.o type.o misc.o hash_table.o scratch.o label.o library.o
	gcc -g -std=c99 library.o scanner.o symbol.o bminor.o parser.o hash_table.o scope.o decl.o stmt.o expr.o param_list.o type.o misc.o -o bminor -lm

bminor.o: bminor.c parser.o
	gcc -g -std=c99 -c bminor.c -o bminor.o

parser.o: parser.c parser.h
	gcc -g -std=c99 -c parser.c -o parser.o

scanner.o: scanner.c
	gcc -g -std=c99 -c scanner.c -o scanner.o

decl.o: decl.c decl.h label.o
	gcc -g -std=gnu99 -c decl.c -o decl.o

stmt.o: stmt.c stmt.h
	gcc -g -c stmt.c -o stmt.o

expr.o: expr.c expr.h
	gcc -g -std=c99 -c expr.c -o expr.o

library.o: library.c library.h
	gcc -g -std=c99 -c library.c -o library.o

param_list.o: param_list.c param_list.h
	gcc -g -std=c99 -c param_list.c -o param_list.o

type.o: type.c type.h
	gcc -g -std=c99 -c type.c -o type.o

misc.o: misc.c misc.h
	gcc -g -std=c99 -c misc.c -o misc.o

scope.o: scope.c scope.h symbol.o
	gcc -g -std=c99 -c scope.c -o scope.o

label.o: label.c label.h
	gcc -g -std=c99 -c label.c -o label.o

scratch.o: scratch.c scratch.h
	gcc -g -std=c99 -c scratch.c -o scratch.o

symbol.o: symbol.c symbol.h
	gcc -g -std=c99 -c symbol.c -o symbol.o

hash_table.o: hash_table.c hash_table.h
	gcc -g -c hash_table.c -o hash_table.o

scanner.c: scanner.flex parser.h
	flex -o scanner.c scanner.flex

parser.c parser.h: parser.bison 
	bison --defines=parser.h --output=parser.c -v -t parser.bison

clean:
	rm -f scanner.c bminor parser.c parser.h parser.output *.o scan
