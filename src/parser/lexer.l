%{
#include "parser.tab.h"  // Link with Bison parser
%}

%option noyywrap

%%

"foreach"          { return FOREACH; }
"in"           { return IN; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); return IDENTIFIER; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"              { return LBRACE; }   /* Handle { */
"}"              { return RBRACE; } 
","             { return COMMA; }
";"             { return SEMICOLON; }
[ \t\r\n]+      { /* Ignore whitespace */ }

.              { printf("Unexpected character: %c\n", yytext[0]); exit(1); }


"/*"([^*]|\*+[^*/])*\*+"/"   { /* Ignore multi-line comments */ }


%%

