# Defining Additional C Grammar for high level mesh manipulation

For the reason of faster code writing and avoiding error while
writing repetitive low level iterators and functions,
it might be a good idea to define few syntaxes related
to mesh mainly for
  * vertex, edge, face iterators,
  * accessing scalars, and other attributes (\_HmeshArray) faster
  * converting high level mathematical expression to low level code, 
and adapt to CUDA/OMP/MPI/AVX etc depending on running config, hardware capabilites, etc.


## Requirement: [flex](https://www.ens-lyon.fr/PSMN/Documentation/modules/flex.html), [bison](https://www.gnu.org/software/bison/manual/bison.html)

flex and bison are required to create a new translator (translating 
'hmesh' grammar to C). flex and bison are NOT required for compiling
the code. Compiling source code require only C99 compiler (gcc recommended).

```bash
sudo apt update
```
```bash
sudo apt install flex bison
```

## Expected 

  * If AST is written to a C file, write it with proper indendation so that it's readable.
  * parser should take care of errors related to extended syntax.

# Read, for better understanding of AST
  
If only couple of simple syntaxes are additionally added to the existing
C grammars, you may be able to convert by direct lexical substitutions.
Otherwise, it is quite complicated because there can be compounded
grammars, local vs global variable scope ambiguity, etc. In this case you 
have to define lexer-parsers that creates 
[abstract syntax tree (AST)](https://en.wikipedia.org/wiki/Abstract_syntax_tree). 
AST generation is an intermediate step  
during code compilation and whoever compile a C code are not required to know it.
However, in our case, AST generation has to be done explicitly
to either convert to native C code, or compile directly to an 
object/executable file.

Intermediate steps involved in converting ".c" source code to binary executable are
  1.  Preprocessing: Substitues and expands macros, include header files. 
  2.  Compilation
```
    [Source Code] → Lexer → Parser → AST/IR (GIMPLE/LLVM IR)
                        ↓               ↓
                   Semantic          Optimizer
                    Check               ↓
                                      Codegen
                                        ↓
                               [Assembly Code, ".s"]
```
  3.  Assembly: Assembler converts assembly code to object file
  4.  Linking: Linker links object files and libraries to produce binary executable

## lexer and parser

A lexer or lexical analyzer reads source code and each lexical tokens are identified.
The lexer for hmesh is defined [lexer.l](./lexer.l)

A parser, meanwhile, takes streams of tokens from the lexer and identifies the grammar
and create an AST. It is quite a difficult job to write a parser code/module as
it involves analysing complex grammar trees. bison can be used to simplify this
job. With the help of a bison grammar file like [parser.y](./parser.y),
bison create parser code. [parser.y](./parser.y) 
has (or will have, once this is done) 
all the C99 grammar and additional grammar for hmesh.
You can create your customized Grammar files 
by building over the standard C grammars like
[C99](https://www.quut.com/c/ANSI-C-grammar-y-1999.html),
[C11](https://www.quut.com/c/ANSI-C-grammar-y-2011.html).

In presence of additional grammars, gcc/llvm lexers and parsers will be
confused and throws error. So user defined lexer and parsers has to do
the job. You can use flex and bison for generating lexer and parser codes
respectively. 

1.  First, run bison on parser.y 
```bash
bison -d -o parser.c parser.y
```
which generates, 
  * parser.c (the parser source code) and 
  * parser.h (the header with each token definition)
2.  Afterwards, run flex on lexer.l
```bash
flex lexer.l
```
which generates 
  * lex.yy.c (the lexer source code), which acts as the token feeder to the parser.
3.  Compile the generated lexer and parser source code along with additional source code, if any.
```bash
gcc lex.yy.c parser.c -o parser
```

## In-house translator

1.  Preprocessor
2.  In-house lexer,parser
  * AST implementation
  * Check grammar
  * Rewrite to a single native C code
3.  Compile using gcc

?? How to check grammar.

?? How to use gcc backend for every other job except lexer/parser.

?? How to take care of preproc

?? How to generate IR.

?? How to show error message for syntax errors related to user-defined grammar.


