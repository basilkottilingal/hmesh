# flex/bison for parsing new rules

For the reason of faster code writing and fewer error,
it might be a good idea to define few syntaxes
mainly for 
  * vertex, edge, face iterators,
  * accessing scalars, and other attributes (\_HmeshArray) faster
  * converting high level  mathematical expression to low level code, and adapt to CUDA/OMP/MPI/AVX etc depending on running config, hardware capabilites, etc.
    
