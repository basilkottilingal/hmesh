# Defined C Grammar for high level mesh manipulation

For the reason of faster code writing and fewer error,
it might be a good idea to define few syntaxes related
to mesh mainly for
  * vertex, edge, face iterators,
  * accessing scalars, and other attributes (\_HmeshArray) faster
  * converting high level mathematical expression to low level code, and adapt to CUDA/OMP/MPI/AVX etc depending on running config, hardware capabilites, etc.


## Requiremennt: flex, bison

flex and bison are required to create a new translator (translating 
hmesh grammar to C). flex and bison are NOT required for compiling
the code. Only required compiling requirement is gcc compiler (C99 std).

$ sudo apt update
$ sudo apt install flex bison

## Expected 

  * In parser, write rules/synataxes with proper indendation, so that
in case bison create a C code, it would be readable.
  * flex/bison should take care of errors related to new synatx before
compiling. 

## Read, for better understanding
  
  Steps in compialtion : token parser (yacc in UNIX, bison in LINUX) + lexer (lex, flex), optimisation, assembly code
  yyparse() : 
  Ast Node :
  bison -d parser.y
  flex lexer.l
  gcc -o parser parser.tab.c lex.yy.c -lm

