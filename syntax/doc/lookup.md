# Identifier lookup

Maintaining a hash-table based stack for identifiers (like function name, variable names, etc..) 
  * Add a new identifier.
    * If already an identifier exist with same name, it's an error. (Scope should be noted);
  * Look for an identifer of some type.
    * If you can't find identifier of sametype, it's an error.

## Reference

some commonly known compiler have implemented the hashtable
  * gcc : [hash-table.h](https://github.com/gcc-mirror/gcc/blob/master/gcc/hash-table.h) based on 
  * clang/llvm : [llvm::stringMap](https://github.com/llvm/llvm-project/blob/main/llvm/lib/Support/StringMap.cpp)
  * [basilisk](http://basilisk.fr/) parser : [faststack.c](http://basilisk.fr/src/ast/faststack.c) based on [khash](https://github.com/attractivechaos/klib/blob/master/khash.h)
