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
  char *             key;
  uint32_t           hash;
} _HashNode;

typedef struct _HashTable {
  _HashNode ** table;
  _AstPool *   pool;
  uint32_t     bits, 
               inuse, 
               threshold;
} _HashTable;

_HashTable _hash_table_ = {0};

void hash_table_init () {
  if(t->table)
    return;

  t->node_pool  = ast_pool (sizeof (_HashNode));
  uint32_t n = 1 << _H_AST_HASHTABLE_SIZE_;
  t->bits = n - 1;
  t->inuse = 0;
  _HashNode ** table = malloc(nnodes * sizeof(_HashNode *));
  if(!table) {
    fprintf(stderr, "hash_table_init() : couldn't create table");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  t->table = table; 
  t->threshold = _H_AST_HASHTABLE_THRESHOLD_ * n;
  while(n--)
    table[n] = NULL; 
}

static int hash_table_resize () {
  _HashTable * t = &_hash_table_;
  assert ( t->table && t->inuse >= t->threshold);
  uint32_t n = t->bits + 1;
  _HashNode ** table = realloc ( 2 * n * sizeof(_HashNode *));
  if(!table)
    return -1;
  
  t->bits = 2 * n - 1;
  t->threshold *= 2;
  t->table = table;
  /*
  .. It's a tricky part and a costly one, goes through the entire 
  .. list of hash nodes. Put all the hash buckets properly at the 
  .. correct index.
  .. ( index = hash % (2*n), where n is the old table size.  )
  */
  _HashNode list;
  _HashNode * head = &list.next;
  *head = NULL;
/*
  for (uint32_t i=0; i<n; ++i) {
    _HashNode * node = table[i];
    while (node) {
      _HashNode * next = node->next; 
      node->next = head;
      head.next  = node;
      node = next;
    }
    table[i] = NULL;
  }

  for (uint32_t i=n; i<2*n; ++i)
    table[i] = NULL;

  while (head.next) {
    _HashNode * node = head.next;
    head.next = head.next->next;
    uint32_t index = node->hash & t->bits;
    _HashNode * next = node->next; 
    node->next = head.next;
    head.next  = node;
    node = next;
  }
    table[i] = NULL;
*/
  
  return 0;
}

static _HashNode * hash_lookup ( const char * key ) {
  _HashTable * t = &_hash_table_;
  uint32_t h = hash ( key );
  uint32_t index = h & t->bits;
  _HashNode * node = t->nodes[index];
  while ( 1 ) {
    if ( h == node->hash )
      if ( !strcmp (h->name, id) )
        return node;
    node = node->next;
  }
  return NULL;
}

static _HashNode * hash_insert ( const char * key ) {

  _HashTable * t = &_hash_table_;
  uint32_t h = hash ( key );
  uint32_t index = h & t->bits;
  _HashNode * node = t->nodes[index];
  while ( 1 ) {
    if ( h == node->hash )
      if ( !strcmp (h->key, key) )
        return node;
    node = node->next;
  }

  node = (_HashNode * ) ast_allocate_from (t->pool);
  node->next = t->nodes[index];
  t->nodes[index] = node;
  node->hash = h;
  node->key  = ast_strdup(id);
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

