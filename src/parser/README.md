# Defined C Grammar for high level mesh manipulation

For the reason of faster code writing and fewer error,
it might be a good idea to define few syntaxes related
to mesh mainly for
  * vertex, edge, face iterators,
  * accessing scalars, and other attributes (\_HmeshArray) faster
  * converting high level mathematical expression to low level code, and adapt to CUDA/OMP/MPI/AVX etc depending on running config, hardware capabilites, etc.


## Requiremennt: flex, bison

flex and bison are required to create a new translator (translating 
'hmesh' grammar to C). flex and bison are NOT required for compiling
the code. Compiling source code require only gcc compiler (C99 std).

$ sudo apt update

$ sudo apt install flex bison

## Expected 

  * In parser, write rules/synataxes with proper indendation, so that
in case bison create a C code, it would be readable.
  * flex/bison should take care of errors related to new synatx before
compiling. 

## Read, for better understanding
  
If only couple of simple syntaxes are additionally added to the existing
C grammars, you may be able to convert by direct lexical substitutions.
Otherwise, it is quite complicated because there can be compounded
grammars, local vs global variable scope ambiguity, etc. In this
case you have to define lexer-parsers that creates 
abstract syntax tree (AST). AST generation is an intermediate step
during code compilation and C coders are not required to know it.
However, in our case, AST generation has to be done explicitly
to either convert to native C code, or compile directly to an 
object/executable file.
  
  Steps in compialtion : token parser (yacc in UNIX, bison in LINUX) + lexer (lex, flex), optimisation, assembly code
  yyparse() : 
  Ast Node :
  bison -d parser.y
  flex lexer.l
  gcc -o parser parser.tab.c lex.yy.c -lm

