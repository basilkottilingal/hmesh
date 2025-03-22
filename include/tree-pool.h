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

#ifndef HMESH_TREE_POOL_DEPTH
/* We restrict the max depth to 3.
.. So all the common datatypes can be
.. represented*/
#define HMESH_TREE_POOL_DEPTH 3
#endif

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
  void * root;

  /* info on each nodes correspodning to memory blocks.
  .. Encodes depth, is a free/used node etc*/
  _Flag * flags;  

  /* size of root. For munmap() */
  size_t size;
} _HmeshTpool;

/* 
.. Allocate a block of 
.. [HMESH_TREE_POOL_SIZE] X [2^d],
.. where d = HMESH_TREE_POOL_SIZE - depth
*/
extern
_Index HmeshTpoolAllocate(_Flag depth); 

/*
.. Get the memory address from starting index
*/
extern
void * HmeshTpoolAddress(_Index block);

/* 
.. Deallocate
*/
extern
_Flag HmeshTpoolDeallocate(_Index block);

/*
.. Free all. @ the end of pgm
*/
extern
void HmeshTpoolDestroy();

#endif
