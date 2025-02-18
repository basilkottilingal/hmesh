#include <stdio.h>
#include <stdlib.h>

#define MEMPOOL 

typedef struct _Memblock _Memblock;

/** 
.. Linked list of free blocks
.*/
typedef struct _FreeBlock {
  /* 'next' to form a linked list of empty blocks
  */
  _FreeBlock * next;

  /* Store information of pool and block number 
  */
  _Memblock memblock; 
} _FreeBlock;

/** 
 * linked list of memory blocks
 */
typedef struct 
_Mempool{
  /* Size of each object. 
  */ 
  size_t object_size; 
   
  /* Total block size.
  .. NOTE : Need to be 64 bit aligned datatype.  
  */
  size_t block_size; 

  /* list of pointers to the memory blocks.
  .. We use an array of block address, rather than 
  .. a linked list for easier access of content, 
  .. Ex : ((double *) pool->blocks[iblock]) [index->i] .
  .. NOTE: Perfer making blocks that align with 2^N.
  */
  void ** blocks; 

  /* Total number of blocks allocated.
  .. NOTE: Limited, nblocks < 256
  */
  _Flag nblocks;
 
  /* Linked list of free blocks.
  .. WARNING: There is no provision to know, if you, ..
  .. deallocate same node multiple times 
  */
  _FreeBlock * free_blocks; 

} _Mempool;

/* Struct to store a particular block of the pool 
*/
struct _Memblock{

  /* which pool
  */
  _Mempool * pool;

  /* which block. iblock \in [0, pool->nblocks)
  */
  _Flag iblock;
};
