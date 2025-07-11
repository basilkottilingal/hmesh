#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <memory.h>
#include <hash.h>

/*
.. MurmurHash3 algorithm (32 bit hash, x86_64 platform).
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

static
_AstPool * pool = NULL;
  
struct _HashTable {
  _HashNode ** table;
  uint32_t n,
           bits, 
           inuse, 
           threshold;
};

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
    case 3: 
      k ^= key[2] << 16;
      #if defined(__GNUC__)  &&  (__GNUC__ >= 7)
        __attribute__((fallthrough));
      #endif
    case 2: 
      k ^= key[1] << 8;
      #if defined(__GNUC__)  &&  (__GNUC__ >= 7)
        __attribute__((fallthrough));
      #endif
    case 1: 
      k ^= key[0];
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

/*
.. Create a _HashTable which stores a hashtable and other metadata. 
*/

_HashTable * hash_table_init ( unsigned int N ) {
  _HashTable * t  = ast_allocate_general (sizeof(_HashTable));

  if(!pool)
    pool = ast_pool ( sizeof (_HashNode) );

  N = N > 20 ? 20 : N;

  uint32_t n = 1 << N;
  _HashNode ** table = malloc(n * sizeof(_HashNode *));
  if(!table) {
    fprintf(stderr, "hash_table_init() : couldn't create table");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  t->n     = n;
  t->bits  = n - 1;
  t->inuse = 0;
  t->table = table; 
  t->threshold = _H_AST_HASHTABLE_THRESHOLD_ * n;
  while(n--)
    table[n] = NULL; 

  return t;
}

/*
void hash_deallocate_node ( _HashNode * node ) {
  ast_deallocate_to ( pool, node );
}
*/

void hash_table_reset ( _HashTable * t ) {
  /*
  .. Cost : O ( n ) |  n = table size. 
  .. fixme : It's not recommended for resetting symbols table when
  .. you pop a scope. 
  */
  int n = t->n;
  _HashNode ** table = t->table;
  _HashNode * h;
  while (n--) {
    while ( ( h = *table ) ) {
      *table = h->next;
      ast_deallocate_to ( pool, h );
    }
    ++table;
  }
}

/*
.. Free the hash table.
.. NOTE : WARNING : _AstPool, nodes created are not freed here.
.. It SHOULD be freed at the end of the program using
.. ast_deallocate_all().
*/
void hash_table_free(_HashTable * t) {
  if(!t) return;
  free(t->table);
}

/*
.. Resize (double) the hash table size.
.. There is a 1 MB limit for table size (excluding nodes).
.. Returns -1 (if failed to expand) or 0 (successful).
*/
static int hash_table_resize (_HashTable * t) {

  assert ( t->table && t->inuse >= t->threshold );
  uint32_t n = t->n, N = n << 1;

  _HashNode ** table = (N > 1<<20) ? NULL :
    realloc ( t->table, N * sizeof(_HashNode *));

  if(!table)
    return -1;

  #ifndef _H_AST_VERBOSE_
    fprintf(stderr, "\nDoubling hash table size");
  #endif
  
  t->n          = N; 
  t->bits      |= n;
  t->threshold *= 2;
  t->table      = table;
  /*
  .. It's a tricky part and a costly one, goes through the entire 
  .. list of hash nodes. Put all the hash buckets properly at the 
  .. correct index.
  .. ( index = hash % (2*n), where 2*n is the new table size.  )
  */

   for (uint32_t i=n; i<N; ++i)
    table[i] = NULL;

  for (uint32_t i=0; i<n; ++i) {
    _HashNode * node = table[i];
    if(!node) continue;

    table[i] = NULL;

    while(node) {
      _HashNode * next = node->next;
      uint32_t _i = node->hash & t->bits;
      /*
      .. move node from index (i % n) to (i % 2n)
      .. where n is the old table size
      */
      node->next = table[_i];
      table[_i] = node;

      node = next;
    }
  }

  return 0;
}

/*
.. look if a key exists
*/

_HashNode * hash_lookup ( _HashTable * t, const char * key ) {

  uint32_t h = hash ( key, strlen(key) );
  uint32_t index = h & t->bits;
  _HashNode * node = t->table[index];
  while ( node ) {
    if ( h == node->hash )
      if ( !strcmp (node->key, key) )
        return node;
    node = node->next;
  }
  return NULL;
}

/*
.. insert a key
*/
_HashNode * hash_insert ( _HashTable * t, const char * key, int symbol ) {

  uint32_t h = hash ( key, strlen(key) );
  uint32_t index = h & t->bits;
  _HashNode * node = t->table[index];
  while ( node ) {
    if ( h == node->hash )
      if ( !strcmp (node->key, key) ) {
        /* 
        .. return NULL if there exists another hash node but with a
        .. different symbol. 
        */
        return node->symbol == symbol ?  node : NULL ;
      }
    node = node->next;
  }

  if(t->inuse == t->threshold) {
    hash_table_resize(t);
    index = h & t->bits;
  }

  node = (_HashNode * ) ast_allocate_from (pool);
  node->next = t->table[index];
  t->table[index] = node;
  node->hash = h;
  node->key  = ast_strdup(key);
  node->symbol = symbol;

  t->inuse++;

  return node;
}
