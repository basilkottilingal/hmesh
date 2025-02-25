#ifndef _HEDGE_MESH_POOL
#define _HEDGE_MESH_POOL

#include <stdio.h>
#include <stdlib.h>

/*
.. Contains datatypes and functions related to pooling/freeing
.. memory blocks to store nodes (like nodes of vertices,
.. edges, etc ..) and memory blocks of scalars corresponding
.. to blocks of node.
..
.. A _Mempool is a collection of blocks of same size.
..   _Mempool
..   ----------   -----------  ..  ----------
..   |        |   |         |  ..  |         |
..   | block-0|   | block-1 |  ..  | block-  |
..   |        |   |         |  ..  |   (N-1) |
..   |        |   |         |  ..  |         |
..   ----------   -----------  ..  ----------
..
.. A _Mempool can be a pool of blocks of nodes or blocks
.. of scalar corresponding to nodes. 
..
.. Block of nodes accomodates MEMBLOCK_SIZE nodes
..    [MEMBLOCK_SIZE x sizeof(_Node)]
..   
.. Block of scalars acommodates MEMBLOCK_SIZE double 
..    [MEMBLOCK_SIZE x double]
..  
.. MEMBLOCK_SIZE is set to 1<<15 by default, it guarantees
.. each block is less than an MB, given sizeof(_Node) <= 32
*/

/*
.. _Memblock stores the info on the pool and the block number.
.. You can access the memory block address using the function
.. void * Memblock(_Memblock);
*/
typedef struct _Memblock _Memblock;

/* 
.. Linked list of free blocks
*/
typedef struct _FreeBlock {
  /* 'next' to form a linked list of empty blocks
  */
  struct _FreeBlock * next;

  /* Store information of pool and block number 
  */
  _Memblock memblock; 

} _FreeBlock;

/* 
.. linked list of memory blocks
*/
typedef struct _Mempool{
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

  /* memory block's address. This information is redundant.
  .. i.e this.address and this.pool->blocks[this.iblock]
  .. are literally same. This is for extra verification.
  void * address;
  */

};

/* 
.. Default number of objs a block stores is 1<<15.
.. You can reset using MemblockSizeReset(size_t nobj).
*/
static 
size_t MEMBLOCK_SIZE 1<<15; 

extern _Flag MemblockSizeReset(size_t nobj);

extern size_t MemblockSize();

/*
.. Creates a new memory pool that holds memory blocks of
.. same size. A pool is a collection of similar blocks of
.. size [MEMBLOCK_SIZE x obj_size]. The function below,
.. creates a pool with 1 free memory block.
*/
extern _Mempool * Mempool(size_t obj_size);

/* 
.. Free a mempool and all the blocks it holds
*/
extern _Flag MempoolFree(_Mempool * pool);

/*
.. Allocate a block from/deallocate a block back to pool
*/
extern _Memblock _MempoolAllocateFrom(_Mempool * pool);
extern _Flag _MempoolDeallocateTo(_Memblock memblock);

#endif
