# flex

## General lexer layout

```
%{
  // C code and includes (optional)
%}

%%
// TOKEN RULES SECTION (regex -> actions)
[0-9]+        { printf("Found a number: %s\n", yytext); }
[a-zA-Z_]+    { printf("Found an identifier: %s\n", yytext); }
"+"           { return PLUS; }
"-"           { return MINUS; }
[ \t\n]       { /* ignore whitespace */ }

%%
// Supporting C code (optional)
```

As in the example, **flex tokens are defined using regular expressions (regex)**
