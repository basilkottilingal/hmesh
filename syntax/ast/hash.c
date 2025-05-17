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
  char * string;
  uint32_t key;
} _HashNode;

typedef struct _HashTable {
  _HashNode ** nodes;
  uint32_t nnodes;
  uint32_t inuse;
  uint32_t threshold;
} _HashTable;


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

