# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Generate parser source
parser.c parser.h: parser.y
	bison $<                          # generating parser source code using bison

# Generate lexical analyser source
lexer.c: lexer.l
	flex -o $@ $<                     # creating flexer source code using 'flex'. output is "lexer.c" 

# parser
parser: parser.c parser.h lexer.c 
	gcc lexer.c parser.c -o $@        # creating parser (executable) "parser"

# parse a .c file 
%.parse: %.c parser
	#gcc -E $< -o $*.i                 # preproc. NOTE: no effect. (preprocessor not yet implemented)
	./parser $<                       # parse the .c input

# Clean up generated files
clean:
	rm -f $(OUTPUT) $(BISON_C) $(BISON_H) $(FLEX_C) output.c


