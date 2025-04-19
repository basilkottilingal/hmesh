# bison

gnu [bison](https://www.gnu.org/software/bison/manual/bison.html) 
is used to create a parser script that produces AST. It is an alternative
to writing a C parser from scratch which is a complex and difficult job to do.

It works along with a tokenizer or lexical analyser code which 
breaks down the source code into tokens. The lexical analyser
can be written in C, or flex can be used to do this easily.

## general bison layout
```bison
%{
/*  Prologue  */
%}

/*  Bison declarations */

%%
/*  Grammar rules  */
%%
/*  Epilogue  */
```

## Grammar example

Grammar rule is defined among **bison declarations**. An example to define
grammar for (minimal) mathematical expression is below
```bison
expr 
  : num
  | expr '+' expr
  | expr '-' expr
  | expr '*' expr
  | expr '/' expr
  | expr '%' expr
  ;
```

## Ast Node

## Adding parameters to functions like yyparse(), yylex(), yyerror()
Add a parameter to ```yylex()``` and ```yyparse()``` respectively
```bison
%lex-param   {yyscan_t scanner}
%parse-param   {AstNode * ast}
```
Add a parameter to all the parser fuctions ```yyparse(), yylex(), yyerror()```
```bison
%parse-param {Node * node}
```

---

## “pure” parser?

In Bison, a **pure parser** is one that doesn’t use any shared global state like:

- `yylval`, `yychar`, or `yylloc`
- `yyparse()` internal static variables

Instead, all necessary data is passed into `yyparse()` through arguments.
This makes it: Thread-safe, Easier to test and modularize, More maintainable for large projects

---

### `full` vs. `true`

| Mode       | What it does                                   |
|------------|------------------------------------------------|
| `true`     | Partially pure — passes `yylval`, but not everything |
| `full`     | Fully pure — all context (scanner state, location, AST root, etc.) must be passed explicitly |

So `%define api.pure full` is saying “Give me full control — I’ll pass all the context manually, and nothing will be global.” The directive:
```bison
%define api.pure full
```
in **Bison** tells the parser to generate a **fully reentrant** (or **pure**) parser — meaning it avoids using **global variables** and instead relies only on **explicit parameters** passed to it.

---

### Example

You must also pass extra parameters using:

```bison
%parse-param {AstRoot *ast}
%parse-param {yyscan_t scanner}
```

And call it like:

```c
yyscan_t scanner;
AstRoot ast;
yylex_init(&scanner);
yyparse(&ast, scanner);
```

## Look-up table
Here’s a handy **cheat sheet** for Bison and Flex global variables.

| Variable     | Defined in | Type         | Purpose                                   |
|--------------|-------------|--------------|-------------------------------------------|
| `yylval`     | Flex → Bison| `YYSTYPE`    | Pass semantic values (e.g., token data)   |
| `yytext`     | Flex        | `char *`     | Matched token text                        |
| `yyin`       | Flex        | `FILE *`     | Input stream (default: `stdin`)           |
| `yyparse()`  | Bison       | `int func()` | Start parser                              |
| `yyerror()`  | User        | `void func()`| Error reporting                           |
| `yychar`     | Bison       | `int`        | Current lookahead token                   |
| `yynerrs`    | Bison       | `int`        | Syntax error count                        |
| `YYSTYPE`    | Bison       | `union/struct`| Semantic value type                       |
| `YYLTYPE`    | Bison       | `struct`     | Source location tracking (if used)        |

