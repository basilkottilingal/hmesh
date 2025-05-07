#ifndef _H_AST_POOL_
#define _H_AST_POOL
  
  #include <stdio.h>
  #include <stdlib.h>

  /*
  .. @ _Free : data type to store a linked list of unused memory chunks
  ..    @ next   : to form a linked list of empty blocks
  ..    @ safety : is an encoded number to make sure,
  ..      you don't allocate or free same block twice
  */
  typedef struct _Free {
    struct _Free * next;
    uint32_t safety;
  } _Free;

  /*
  .. @ _MemoryHandle : Datatype that stores all root information 
  ..    related to memory handling
  */
  typedef struct {
    
  }

  /*
  .. @ _Block : Datatype for a memory block which is ususally a big chunk
  ..  of memory of the order of MB. It's data members are
  ..    @ address : memory address. Can be used to free.
  ..    @ size    : size of the memory chunk.
  ..    @ fhead   : head of the free list
  */
  typedef struct {
    void * address;
    size_t size;
    _Free * fhead;
  } _Block;
 
  /*
  .. A page 
  */ 
  typedef struct {
    void * pool;
    _Index iblock;
  } _Page;
  
  /* 
  .. Linked list of free blocks
  */
  typedef struct _FreeBlock {
    /* 'next' to form a linked list of empty blocks
    */
    struct _FreeBlock * next;
  
    /* 'safety' is an encoded number to make sure,
    .. you don't allocate or free same block twice
    */
    uint32_t safety;
  
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
    void ** address; 
   
    /* Linked list of free blocks.
    .. WARNING: There is no provision to know, if you, ..
    .. deallocate same node multiple times 
    */
    _FreeBlock * free_blocks; 
  
    /* Total number of blocks allocated.
    .. NOTE: Limited, 0 <= nblocks <= 256
    int nblocks;
    */
  
    /* keeping track of used/free block indices 'iblock' */
    _IndexStack stack;
  
  } _Mempool;
  
  
  /* Reset the number of nodes per block */
  extern _Flag MemblockSizeReset(size_t nobj);
  
  /* get the number of nodes per block */
  extern size_t MemblockSize();
  
  /* Get the memroy address of the block */
  extern void * MemblockAddress(_Memblock block);
  
  /*
  .. Creates a new memory pool that holds memory blocks of
  .. same size. A pool is a collection of similar blocks of
  .. size [_MEMBLOCK_SIZE_ x obj_size]. The function below,
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
  extern _Memblock MempoolAllocateFrom(_Mempool * pool);
  extern _Flag MempoolDeallocateTo(_Memblock memblock);
  
  /*
  .. A stack of memory pool.
  .. NOTE : Restricted to obj_size in [1,2,4,8,16,32].
  .. It can be easily achieved  if you follow Structure 
  .. of Array (SoA) for databases.
  .. NOTE : It is not a sophisticated algo as 
  .. there is no coalascence/fragmentation of 
  .. memory blocks. So deallocated blocks (not yet freed)
  .. can be wasteful and malloc can fail for memblocks of
  .. different blocksizes. So it is advised to free memblocks
  .. if they have accumulated.
  .. ALTERNATIVE : A tree pool
  .. can be implemented since block_size follow 2^N.
  */
  
  extern _Memblock MempoolAllocateGeneral(size_t obj_size);
  
  extern _Flag MempoolDeallocateGeneral(_Memblock);
  
  extern _Flag MempoolFreeGeneral();
  
#endif
