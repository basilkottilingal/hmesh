/*
.. Large chunk memory of order of ~8MB is allocated using mmap().
.. It is used as a memory pool for arrays of [BLOCK_SIZE * OBJ_SIZE],
.. where BLOCK_SIZE is fixed, and OBJ_SIZE in {2^N} := {1,2,4,8} 
.. (in worst case scenarios you can expand this to {1,2,4,8,16,32}).
.. When you use SoA (Structure of Arrays of basic datatypes),you can 
.. make sure that object_size are in {1,2,4,8 bytes} for a 64 bit 
.. processor. So a tree allocator can be used to allocate memory 
.. chunks BLOCK_SIZE x 2^N, by repetitively dividing N times.
.. For a max depth of 3 (0<= N <= 3), there will be minimal overhead 
.. in alloc/deallocate with divide/coalesce.
*/

#ifndef HMESH_TREE_POOL
#define HMESH_TREE_POOL

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

#ifndef HMESH_TREE_POOL_DEPTH
/* We restrict the max depth to 3.
.. So all the common datatypes can be
.. represented*/
#define HMESH_TREE_POOL_DEPTH 3
#endif
const size_t HMESH_TREE_POOL_NODES = 
  (1 << (HMESH_TREE_POOL_DEPTH+1)) - 1;

/* For the moment, let's fix the BLOCK_SIZE = no: of
.. nodes per block to 4096. So the the size of chunks
.. fall in 4096 * {1,2,4,8}, guaranteeing the minimum
.. of the chunk size is a PAGE_SIZE = 4096 Bytes.*/
const size_t HMESH_TREE_BLOCK_SIZE = 1<<12;

/* We try to allocate 8MB of chunk */
#define HMESH_TREE_POOL_SIZE 1<<23



/* 
.. Linked list of free blocks
*/
typedef struct _FreeTBlock {
  /* 'next' to form a linked list of empty blocks
  */
  struct _FreeTBlock * prev, * next;

  /* Store information of itree, inode
  */
  _Index block; 

} _FreeTBlock;

typedef struct {
  /* address of each root node */
  void ** root;

  /* info on each block. (a) is_used or is_free*/
  _Flag ** info;  

  /* number of trees in use */
  _Flag ntrees; 

  /* Free blocks of each level */
  _FreeBlock * free_list[HMESH_TREE_POOL_DEPTH + 1]; 

} _HmeshTreepool;


#endif
