# flex/bison for parsing new rules

For the reason of faster code writing and fewer error,
it might be a good idea to define few syntaxes related
to mesh mainly for
  * vertex, edge, face iterators,
  * accessing scalars, and other attributes (\_HmeshArray) faster
  * converting high level mathematical expression to low level code, and adapt to CUDA/OMP/MPI/AVX etc depending on running config, hardware capabilites, etc.


## Requiremennt: flex, bison

$ sudo apt update
$ sudo apt install flex bison

## Expected 

  * In parser, write rules/synataxes with proper indendation, so that
in case bison create a C code, it would be readable.
  * flex/bison should take care of errors related to new synatx before
compiling. 
