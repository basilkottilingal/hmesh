#ifndef _H_AST_HASH_
#define _H_AST_HASH_
  
  /*
  .. String Hashing :
  .. - Uses murmurhash3 for string hashing.
  .. - HashTable uses bucket linked list to handle collision
  .. - Resizes hashtable when load reaches _H_AST_THRESHOLD_
  .. - NOTE : WARNING : key (char *) will fail to store, if
  ..   strlen >= 4096 which can mess the entire program. 
  ..
  ..
  .. Read More :
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

  #include <stdint.h>

  #include <memory.h>

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

  typedef struct _HashNode {
    struct _HashNode * next;
    char *             key;
    uint32_t           hash;
  } _HashNode;

  typedef struct _HashTable {
    _HashNode **       table;
    _AstPool *         pool;
    uint32_t           bits, 
                       inuse, 
                       threshold;
  } _HashTable;

  /* 
  .. following are the api functions.
  .. (a) create a hash table with slot/index size == _H_AST_TABLESIZE_
  .. (b) to insert a node whose key is the string 'key' 
  .. (c) to look for  a node with key 'key' 
  .. (d) delete all key data related to the table.
  ..     NOTE : WARNING: the pool created for the hash nodes will
  ..     survive till you destruct all the memory blocks at the end
  ..     of the progrma using
  ..     ast_deallocate_all();
  */
  extern _HashTable *  hash_table_init();
  extern _HashNode *   hash_insert ( _HashTable *, const char * key);
  extern _HashNode *   hash_lookup ( _HashTable *, const char * key);
  extern void          hash_table_free ( _HashTable * );

#endif
