#ifndef _H_AST_HASH_
#define _H_AST_HASH_
  
  /*
  .. In the context of parser, you have to avoid redundant
  .. creation of same identifier (func name, variable name, ..) node.
  .. In order to implement this, each time you encounter an identifier
  .. name, you have to see if identifier with same name already exists
  .. node with the same name. This can be efficienty implemented
  .. by a string hashing algorithm.
  ..
  .. String hashing itself is an area of great interest & research.
  .. In non-cryptographic hashing as in our context, the main factors that
  .. influence the choice of hashing are speed vs hash collision vs memory.
  .. The design factor also includes load factor (ratio of expected number 
  .. of string to the availale hash table size). Few of the most
  .. commonly used algorithms are :
  ..  | ------------------- | ------------------------------------- |
  ..  | Compiler / Language | Hash Function(s) Used                 |
  ..  | ------------------- | ------------------------------------- |
  ..  | GCC                 | Custom (djb2-like, cpp\_hash\_string) |
  ..  | Clang / LLVM        | MurmurHash3, xxHash                   |
  ..  | Python (CPython)    | SipHash (since 3.3)                   |
  ..  | Java (HotSpot)      | Polynomial Rolling (31ᵏ)              |
  ..  | JavaScript (V8)     | FNV-1a, MurmurHash, custom            |
  ..  | Rust                | SipHash, FxHash                       |
  ..  | Perl                | Polynomial, Randomized                |
  ..  | Go                  | FNV/AHash, custom                     |
  ..  | Haskell (GHC)       | SipHash, MurmurHash2                  |
  ..  | ------------------- | ------------------------------------- |
  ..
  .. Few of the capabilities that you expect by the string hashing algo :
  ..  - Handle hash collision :  buckets vs open addressing (closed hashing)
  ..  - Resize the table if table load factor reaches (say) 0.75
  ..
  */

  /* 
  .. default size of the hash table is 2^12 hash nodes. 
  .. If set by user, make sure size is in [2^8, 2^20]
  */
  #ifndef   _H_AST_HASHTABLE_SIZE_
    #define _H_AST_HASHTABLE_SIZE_   12      /* 2^12 = 4096 */
  #elif     _H_AST_HASHTABLE_SIZE_ > 20
    #undef  _H_AST_HASHTABLE_SIZE_ 
    #define _H_AST_HASHTABLE_SIZE_   20      /* 2^20 */
  #elif     _H_AST_HASHTABLE_SIZE_ < 8
    #undef  _H_AST_HASHTABLE_SIZE_ 
    #define _H_AST_HASHTABLE_SIZE_   8       /* 2^8 */
  #endif

  /*
  .. Threshold load of hash table, after which 
  .. algo will try to double the table size.
  */
  #ifndef _H_AST_HASHTABLE_THRESHOLD_
    #define _H_AST_HASHTABLE_THRESHOLD_ 0.75
  #endif

  /* 
  .. following are the api functions 
  */
  extern void * ast_hash_insert ( const char * );
  extern void * ast_hash_search ( const char * );

#endif
