#include <common.h>
#include <mempool.h>

void * MemblockAddress(_Memblock memblock) {
  /* Get the block address from a _Memblock object 
  */

  _Mempool * pool = (_Mempool *) memblock.pool;

  _Flag iblock = memblock.iblock;
  /* The case "iblock >= pool->nblocks" should never happen
  */
  void * block_address = !pool ? NULL :
    (iblock >= pool->nblocks) ? NULL :  
    pool->blocks[iblock];

  return block_address;
}

/* 
.. Default number of objs a block stores is 1<<15.
.. You can reset using MemblockSizeReset(size_t nobj).
*/
static 
size_t _MEMBLOCK_SIZE_ = 1<<15;

_Flag MemblockSizeReset(size_t new_nobj) {
  if(new_nobj > (1<<16))
    return 0;
  _MEMBLOCK_SIZE_ = new_nobj;
  return 1; 
}

size_t MemblockSize() {
  return _MEMBLOCK_SIZE_;
}

_Mempool * Mempool(size_t object_size) {
      
  _Mempool * pool =  (_Mempool*)malloc(sizeof(_Mempool));

  size_t block_size = MemblockSize() * object_size;

  if(block_size < sizeof (_FreeBlock)){
    /* In this case you might overwrite next used block 
    .. when typecasting empty blocks to _FreeBlock * 
    */
    HmeshError("Mempool() : Inadequate block size"); 
    return NULL;
  }

  if(block_size > 1<<20) {
    /* 1MB is the capped limit. You can't override this limit.
    .. Rounding off the block size to 1MB might destroy the 
    .. alignment of number of indices per block. So returning.
    */
    free(pool);
    HmeshError("Mempool() : Block size greater than 1MB"); 
    return NULL;
  }

  if(block_size % 8) {
    /* You can easily achive this by taking nobjects = 1<<15,
    .. assuming object_size <= 32 Bytes. 
    */
    free(pool);
    HmeshError("Mempool() : Bad block size alignment"); 
    return NULL;
  }

  /* Creating one memory block in the pool
  */
  void * address = malloc(block_size);
  if (!address) {
    free(pool);
    HmeshError("Mempool() : Couldn't create Memory Block"); 
    return NULL;
  }
 
  /* Setting pool object's fields */
  pool->object_size = object_size;
  pool->block_size  = block_size;
  pool->nblocks = 1;
  pool->blocks = (void **) calloc (1, sizeof(void *));
  /* The only available block in the pool, as of now */
  pool->blocks[0] = address; 
  /* The free blocks linked list */
  _FreeBlock * fb = (_FreeBlock *) address;
  fb->next = NULL;
  /* Encode this safety number to avoid double freeing */
  fb->safety = 0xFBC9183;
  fb->memblock = (_Memblock) {.pool = pool,.iblock = 0};
  pool->free_blocks = fb;

#ifndef _HMESH_VERBOSE_OFF
  fprintf(stdout, "\n1 x Pool Created with \n1 x MemBlock "
    "[%ld Bytes = %ld x %ld Bytes]",
    pool->block_size, MemblockSize(), pool->object_size);
  fflush(stdout);
#endif

  return pool;
}

_Memblock MempoolAllocateFrom(_Mempool * pool) {
  /* Allocate a memory bloc  from the pool of blocks */

  if (!pool){
    HmeshError("MempoolAllocateFrom() : No pool Mentioned");
    return (_Memblock) {.pool = NULL, .iblock = 0};
  }

  if (!pool->free_blocks && pool->nblocks == UINT8_MAX) {
    HmeshError("MempoolAllocateFrom() : Out of blocks");
    return (_Memblock) {.pool = NULL, .iblock = 0};
  }

  /* Send a free block {pool + iblock}  if available 
  */
  if(pool->free_blocks) {
    _FreeBlock * fb = pool->free_blocks;
    if(fb->safety != 0xFBC9183) {
      HmeshError("MempoolAllocateFrom() : Double allocation"); 
      return (_Memblock) {.pool = NULL, .iblock = 0};
    }
    fb->safety = 0xC1C2A9F;  
    pool->free_blocks = fb->next;
    return fb->memblock;
  }

  /* create a new block as there is no free blocks available
  */
  void * address = malloc(pool->block_size);
  if (!address) {
    free(pool);
    HmeshError("MempoolAllocateFrom() : Cannot create block");
    return (_Memblock) {.pool = NULL, .iblock = 0};
  }
  ((_FreeBlock *)address)->safety = 0xC1C2A9F;

  ++(pool->nblocks);
  pool->blocks = (void **) realloc ( pool->blocks,
    pool->nblocks * sizeof (void *));

  _Flag iblock = pool->nblocks - 1;
  pool->blocks[iblock] = address;

  return (_Memblock) {.pool = pool, iblock = iblock};
}

/* Deallocate memory back to the pool
*/
_Flag MempoolDeallocateTo(_Memblock memblock) {

  _Mempool * pool = (_Mempool *) memblock.pool;
  void * address = MemblockAddress(memblock);

  if ( !address ) {
    HmeshError("MempoolDeallocateTo() : block not mentioned");
    fflush(stderr);
    return _HMESH_ERROR;
  }

  /* Add this block back to the free list of blocks */
  _FreeBlock * fb = (_FreeBlock*) address;
  fb->next = pool->free_blocks;
  if(fb->safety == 0xFBC9183) {
    HmeshError("MempoolDeallocateTo() : double freeing");
    return _HMESH_ERROR;
  }
  fb->safety = 0xFBC9183;
  fb->memblock = memblock;
  pool->free_blocks = fb;
  
  /* success */
  return _HMESH_NO_ERROR;
}

/* Destroy pool*/
_Flag MempoolFree(_Mempool * pool) {
  if (!pool) { 
    HmeshError("MempoolFree() : pool not mentioned");
    fflush(stderr);
    return _HMESH_ERROR;
  }

  _Flag status = _HMESH_NO_ERROR;
  for(_Flag i=0; i<pool->nblocks; ++i) {
    _FreeBlock * fr = (_FreeBlock *) pool->blocks[i];
    if(fr->safety != 0xFBC9183) {
      /* Warning. Does not interrrupt */
      HmeshError("MempoolFree() : Warning : "
        "block might still be in use");
      status = _HMESH_ERROR;
    }
    free(pool->blocks[i]);
  }
  free(pool->blocks);
  free(pool);
  
  return status;
}
