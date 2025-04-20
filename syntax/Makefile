# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Generate parser using bison
parser.c parser.h: parser.y
	bison $<

# Generate lexical analyszer using flex
lexer.c: lexer.l
	flex -o $@ $<
	
#flex -o $@ $< -lfl // -lfl for flex library .

# parser
parser: parser.c parser.h lexer.c 
	gcc lexer.c parser.c -o $@

# parse a .c file 
%.parse: %.c parser
	gcc lexer.c parser.c -o parser
	./parser $<

# Clean up generated files
clean:
	rm -f $(OUTPUT) $(BISON_C) $(BISON_H) $(FLEX_C) output.c


