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
  .. Threshold load of hash table, after which 
  .. algo will try to double the table size.
  */
  #ifndef _H_AST_HASHTABLE_THRESHOLD_
    #define _H_AST_HASHTABLE_THRESHOLD_ 0.75
  #endif

  typedef struct _HashNode {
    struct _HashNode * next;
    const char *       key;
    uint32_t           hash;
    int                symbol;
  } _HashNode;

  typedef struct _HashTable _HashTable;

  /* 
  .. following are the api functions.
  .. (a) create a hash table with slot or index size = 2^N
  .. (b) to insert a node whose key is the string 'key' & symbol is 'sym' 
  .. (c) to look for a node with key 'key' & symbol 'sym'
  .. (d) delete all key data related to the table.
  ..     NOTE : WARNING: the pool created for the hash nodes will
  ..     survive till you destruct all the memory blocks at the end
  ..     of the progrma using
  ..     ast_deallocate_all();
  .. (e) deallocate a hash node to pool ( & reuse later )
  */
  extern _HashTable *  hash_table_init( unsigned int N );
  extern _HashNode *   hash_insert ( _HashTable *, const char * key, int sym );
  extern _HashNode *   hash_lookup ( _HashTable *, const char * key );
  extern void          hash_table_free ( _HashTable * );
  extern void          hash_deallocate_node ( _HashNode * );

#endif
