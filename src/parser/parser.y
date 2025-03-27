%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(const char *s);
int yylex();
extern int yylineno;
extern FILE *yyin;  // Allow reading from a file

FILE *outfile; // File pointer for output.c

%}

%union {
    char *str;
}

%token <str> FOREACH IN LPAREN RPAREN COMMA IDENTIFIER SEMICOLON LBRACE RBRACE

%%

program:
    stmt_list
    ;

stmt_list:
    stmt stmt_list
    | /* empty */
    ;

stmt: 
    loop_stmt
    | block_stmt
    | simple_stmt
    ;

loop_stmt:
    FOREACH LPAREN IDENTIFIER COMMA IDENTIFIER RPAREN IN IDENTIFIER block_stmt
    {
        fprintf(outfile, "iterator = %s.head;\n", $8);
        fprintf(outfile, "while (iterator) {\n");
        fprintf(outfile, "    %s = iterator.a;\n", $3);
        fprintf(outfile, "    %s = iterator.b;\n", $5);
        fprintf(outfile, "    /* loop body */\n");
        fprintf(outfile, "    iterator = iterator.next;\n");
        fprintf(outfile, "}\n");
    }
    ;

block_stmt:
    LBRACE stmt_list RBRACE
    ;

simple_stmt:
    IDENTIFIER SEMICOLON
    {
        fprintf(outfile, "%s;\n", $1);
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s input_file\n", argv[0]);
        return 1;
    }

    // Open input file
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Error opening input file");
        return 1;
    }

    // Open output file
    outfile = fopen("output.c", "w");
    if (!outfile) {
        perror("Error opening output file");
        fclose(yyin);
        return 1;
    }

    yyparse();  // Start parsing

    fclose(yyin);
    fclose(outfile);
    printf("Parsing complete. Output saved to output.c\n");

    return 0;
}

