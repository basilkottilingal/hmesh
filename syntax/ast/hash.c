#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <memory.h>
#include <hash.h>

/* 
.. @ _HashNode : struct that store hash node for string hashing.
..    We maintain linked list of hash bucket to handle hash collision.
*/

typedef struct _HashNode {
  struct _HashNode * next;
  char * key;
  uint32_t hash;
} _HashNode;

typedef struct _StringPool {
  char * page;
  struct _StringPool * prev;
  size_t index;
  size_t limit;
}

typedef struct _HashTable {
  _HashNode ** table;
  _AstPool * pool;
  uint32_t bits;
  uint32_t inuse;
  uint32_t threshold;
} _HashTable;

_HashTable _hash_table_ = {0};

void hash_table_init () {
  if(t->nodes)
    return;

  t->node_pool  = ast_pool (sizeof (_HashNode));
  uint32_t nnodes = 1 << _H_AST_HASHTABLE_SIZE_;
  t->bits       = nnodes - 1;
  t->inuse      = 0;
  _HashNode ** nodes = malloc(nnodes * sizeof(_HashNode *));
  if(!nodes) {
    fprintf(stderr, "hash_table_init() : couldn't create table");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  t->nodes      = nodes; 
  t->threshold  = _H_AST_HASHTABLE_THRESHOLD_ * nnodes;
  while(nnodes--)
    t->nodes[nnodes] = NULL; 
}

static int hash_table_resize () {
  _HashTable * t = &_hash_table_;
  assert ( t->nodes && t->inuse >= t->threshold);
  uint32_t nnodes = t->bits + 1;
  _HashNode ** nodes = realloc ( 2 * nnodes* sizeof(_HashNode *));
  if(!nodes)
    return -1;
  
  t->bits |= nnodes;
  t->threshold *= 2;
  return 0;
}

static _HashNode * hash_insert ( const char * id ) {
  _HashTable * t = &_hash_table_;
  uint32_t h = hash ( id );
  uint32_t index = h & t->bits;
  _HashNode * node = t->nodes[index];
  while (node) {
    if (h == node->hash)
      return node;
    if( !strcmp (h->name, id) )
      return node;
    node = node->next;
  }
  _HashNode * new_node = (_HashNode * ) ast_allocate_from (t->pool);
  new_node->next = t->nodes[index];
  t->nodes[index] = new_node;
  new_node->hash = h;
}

/*
.. MurmurHash3 algorithm.
.. 
.. Credit : Austin Appleby, (MIT License)
.. https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
..
.. @ params  
..    - key : identifier string             (char *)
..    - len : length of string, use yyleng. (uint32_t)
.. @ return 
..    - hash : uint32_t
*/

static inline 
uint32_t hash ( const char * key, uint32_t len ) {
  	
  /* 
  .. Seed is simply set as 0
  */
  uint32_t h = 0;
  uint32_t k;

  /*
  .. Dividing into blocks of 4 characters.
  */
  for (size_t i = len >> 2; i; --i) {
    memcpy(&k, key, sizeof(uint32_t));
    key += sizeof(uint32_t);

    /*
    .. Scrambling each block 
    */
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;

    h ^= k;
    h = ((h << 13) | (h >> 19)) * 5 + 0xe6546b64;
  }

  /*
  .. tail characters.
  .. This is for little-endian.  You can find a different version 
  .. which is endian insensitive.
  */  
  k = 0; 
  switch (len & 3) {
    case 3: k ^= key[2] << 16;
    case 2: k ^= key[1] << 8;
    case 1: k ^= key[0];
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
  }

  /* 
  .. Finalize
  */
  h ^= len;
  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  /* return murmur hash */
  return h;
}

static inline 
_HashNode * insert ( const char * s ) {
  
}

static void hash_table_expand (void) {
  
}

