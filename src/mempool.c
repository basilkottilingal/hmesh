#include <mempool.h>

void * Memblock(_Memblock memblock) {
  /* Get the block address from a _Memblock object 
  */

  _Mempool * pool = memblock.pool;

  _Flag iblock = memblock.iblock;
  /* The case "iblock >= pool->nblocks" should never happen
  */
  void * block_address = !pool ? NULL :
    (iblock >= pool->nblocks) ? NULL :  
    pool->blocks[iblock];

  return block_address;
}

_Mempool * Mempool(size_t object_size, size_t nobjects) {
    
  _Mempool * pool =  (_Mempool*)malloc(sizeof(_Mempool));

  size_t block_size = nobjects * object_size;

  if(block_size < sizeof (FreeBlock)){
    /* In this case you might overwrite next used block 
    .. when typecasting empty blocks to _FreeBlock * 
    */
    fprintf(stderr, 
      "\nError : block size should be atleast %ld ", 
      sizeof(FreeBlock));
    fflush(stderr);  
    return NULL;
  }

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
      "\nError : Prefer a multiple of 64 bits for memBlock.");
    fflush(stderr);  
    return NULL;
  }

  /* Creating one memory block in the pool
  */
  void * address = malloc(block_size);
  if (!address) {
    free(pool);
    fprintf(stderr, 
      "\nError : Failed to allocate memory for the memblock");
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
  /* The free blocks linked list */
  _FreeBlock * fb = (_FreeBlock *) address;
  fb->next = NULL;
  fb->memblock = (_Memblock) {.pool = pool, .iblock = 0};
  pool->free_blocks = fb;

#ifdef _MANIFOLD_DEBUG
  fprintf(stdout, "\n1 x Pool Created with 1 x MemBlock ");
  fprintf(stdout, "[%ld bytes = %ldx%ld]"
    pool->block_size, nobjects, pool->object_size);
  fflush(stdout);
#endif

  return pool;
}

_Memblock MempoolAllocateFrom(_Mempool * pool) {
  /* Allocate a memory bloc  from the pool of blocks */

  if (!pool){
    fprintf(stderr, "Error : No pool Mentioned!");
    fflush(stderr);
    return NULL;
  }

  if (!pool->free_blocks && pool->nblocks == UINT8_MAX) {
    fprintf(stderr, "\nWarning : No free blocks available.");
    fflush(stderr);
    return NULL;
  }

  /* Send a free block {pool + iblock}  if available 
  */
  if(pool->free_blocks) {
    FreeBlock * fb = pool->free_blocks;
    pool->free_blocks = fb->next;
    return fb->memblock;
  }

  /* create a new block as there is no free blocks available
  */
  void * address = malloc(pool->block_size);
  if (!address) {
    free(pool);
    fprintf(stderr, 
      "\nError : Failed to allocate memory for a new memblock");
    fprintf(stderr," in the pool");
    fflush(stderr);  
    return NULL;
  }

  ++(pool->nblocks);
  pool->blocks = (void **) realloc ( pool->blocks,
    pool->nblocks * sizeof (void *));

  _Flag iblock = pool->nblocks - 1;
  pool->blocks[iblock] = address;

  return (_Memblock) {.pool = pool, .iblock = iblock};
}

/* Deallocate memory back to the pool
*/
_Flag MempoolDeallocateTo(_Memblock memblock) {

  _Mempool * pool = memblock.pool;
  void * block = Memblock(memblock);

  if (! (pool && block) ) {
    fprintf(stderr, 
      "Error : Pool/Block not mentioned!");
    fflush(stderr);
    return 0;
  }

  /* Add this block back to the free list of blocks */
  _FreeBlock * fb = (_FreeBlock*) address;
  fb->next = pool->free_blocks;
  fb->memblock = memblock;
  pool->free_blocks = fb;
  
  /* success */
  return 1;
}

/* Destroy pool*/
_Flag MempoolFree(_Mempool * pool) {
  if (!pool) 
    return 0;

  for(_Flag i=0; i<pool->nblocks; ++i)
    free(pool->blocks[i]);
  free(pool->blocks);
  free(pool);
  
  return 1;
}
