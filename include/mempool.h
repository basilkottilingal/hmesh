#include <stdio.h>
#include <stdlib.h>

#define MEMPOOL 

/** 
.. Linked list of free blocks
.*/
typedef struct _FreeBlock {
  _FreeBlock * next; 
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

_Mempool * Mempool(size_t object_size, size_t nobjects) {
  // Initialize the memory pool
  // FIXME: Redo this:

  if(object_size < sizeof (void *)){
    /* In this case you might overwrite next used nodes 
    .. when typecasting empty memspaces to _FreeBlock * 
    */
    fprintf(stderr, 
      "\nError : Oject size should be atleast %ld ", 
      sizeof(void *));
    fflush(stderr);  
    return NULL;
  }
    
  _Mempool * pool =  (_Mempool*)malloc(sizeof(_Mempool));

  size_t block_size = nobjects * object_size;
  if(block_size > 1<<20) {
    /* 1MB is the capped limit. You can't override this limit.
    .. Rounding off the block size to 1MB might destroy the 
    .. alignment of number of indices per block. So returning.
    */
    free(pool);
    fprintf(stderr, 
      "\nError : NOBJECTS x OBJ_SIZE exceeds 1 MB");
    fflush(stderr);  
    return NULL;
  }

  if(block_size % 8) {
    /* You can easily achive this by taking nobjects = 1<<15,
    .. assuming object_size <= 32 Bytes. 
    */
    free(pool);
    fprintf(stderr, 
      "\nError : Prefer a multiple of 64 bit for object size. ");
    fflush(stderr);  
    return NULL;
  }

  /* Creating one memory block in the pool
  */
  void * address = malloc(block_size);
  if (!address) {
    free(pool);
    fprintf(stderr, 
      "\nError : Failed to allocate memory for the memory block");
    fprintf(stderr," in the pool");
    fflush(stderr);  
    return NULL;
  }
 
  /* Setting pool object's fields */
  pool->object_size = object_size;
  pool->block_size  = block_size;
  pool->nblocks = 1;
  pool->blocks = (void **) calloc (1, sizeof(void *));
  /* The one and only block in the pool */
  pool->blocks[0] = address; 
  /* The free node linked list */
  _FreeBlock * fb = (_FreeBlock *) address;
  address->next = NULL;
  pool->free_blocks = fb;

#ifdef _MANIFOLD_DEBUG
  fprintf(stdout, "\n1 x Pool Created with 1 x MemBlock ");
  fprintf(stdout, "[%ld bytes = %ldx%ld]"
    pool->block_size, nobjects, pool->object_size);
  fflush(stdout);
#endif

  return pool;
}

// Allocate memory from the pool
void * MempoolAllocateFrom(_Mempool * pool) {
  if (!pool){
    fprintf(stderr, "Error : No pool Mentioned!");
    fflush(stderr);
    return NULL;
  }

  if (!pool->free_blocks && pool->nblocks == UINT8_MAX) {
    fprintf(stderr, "\nWarning : No free blocks available.");
    fprintf(stderr, " Cannot create another block");
    fflush(stderr);
    return NULL;
  }

  /* Send a freeBlock if available 
  */
  if(pool->free_blocks) {
    FreeBlock * block = pool->free_blocks;
    pool->free_blocks = block->next;
    return (void *) block;
  }

  /* create a new block
  */
  void * address = malloc(pool->block_size);
  if (!address) {
    free(pool);
    fprintf(stderr, 
      "\nError : Failed to allocate memory for a new memory block");
    fprintf(stderr," in the pool");
    fflush(stderr);  
    return NULL;
  }

  ++(pool->nblocks);
  pool->blocks = (void **) realloc ( pool->blocks,
    pool->nblocks * sizeof (void *));
  pool->blocks[pool->nblocks - 1] = address;

  return address;
}

/* Deallocate memory back to the pool
*/
_Flag MempoolDeallocateTo(_Mempool * pool, void * block) {
  if (!pool || !block) {
    fprintf(stderr, 
      "Error : Pool/Block not mentioned!");
    fflush(stderr);
    return 0;
  }

  /* Add this block back to the free list of blocks */
  _FreeBlock * fb = (_FreeBlock*) block;
  fb->next = pool->free_blocks;
  pool->free_blocks = fb;
  
  /* success */
  return 1;
}

/* Destroy pool*/
_Flag MempoolFree(_Mempool *pool) {
  if (!pool) 
    return 0;

  for(_Flag i=0; i<pool->nblocks; ++i)
    free(pool->blocks[i]);
  free(pool->blocks);
  free(pool);
  
  return 1;
}
