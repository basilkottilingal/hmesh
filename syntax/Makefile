# Compiler and flags
CC			 = gcc
CFLAGS	 = -Wall -Wextra
CFLAGS  += -I./ast
#CFLAGS += -L./ast -last

# Generater parser.y ( with ast ) from grammar/parser.y ( simpler )
parser.y: grammar/parser.y grammar/rules.h grammar/tree.c
	cd grammar && make rules 

# Generate parser source
parser.c parser.h: parser.y
	bison $<						# generating parser source code using bison

# Generate lexical analyser source
lexer.c: lexer.l
	flex -o $@ $<			 # creating flexer source code using 'flex'. output is "lexer.c" 

ast/libast.a:
	cd ast/ && make 

# parser
parser: parser.c parser.h lexer.c ast/libast.a
	$(CC) $(CFLAGS) lexer.c parser.c ./ast/libast.a -o $@			 # creating parser (executable) "parser"

# parse a .c file 
%.parse: %.c parser ast/libast.a
	#gcc -std=c99 -E $< -o $*.i		# preproc. NOTE: no effect. (preprocessor not yet implemented)
	./parser $<				 # parse the .c input

all: parser.y parser.c lexer.c ast/libast.a parser

# Clean up generated files
clean:
	rm -f parser parser.h parser.c lexer.c


