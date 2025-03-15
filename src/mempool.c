#include <common.h>
#include <mempool.h>

void * MemblockAddress(_Memblock memblock) {
  /* Get the block address from a _Memblock object 
  */

  _Mempool * pool = (_Mempool *) memblock.pool;

  int iblock = memblock.iblock;
  /* The case "iblock >= pool->nblocks" should never happen
  */
  void * address = !pool ? NULL :
    (iblock >= pool->stack.max) ? NULL :  
    pool->address[iblock];

  return address;
}

/* 
.. Default number of objs a block stores is 1<<15.
.. (NOT the size of block in bytes)
.. You can reset using MemblockSizeReset(size_t nobj).
.. Maximum allowed no: nodes/block = 1<<16 = 1 + UINT16_MAX
*/
static size_t HMESH_MEMBLOCK_SIZE = 1<<15;

_Flag MemblockSizeReset(size_t new_nobj) {
  /* fixme: Once some pool is already created, this function 
  .. should return error, otherwise different blocks will
  .. have different sizes 
  */

  if ( (new_nobj < (1<<5)) || (new_nobj > (1<<15)) ) {
    HmeshError("MemblockSizeReset() : aborted. "
      "preferred nobj = 2^N with N in [5, 6, .., 15]");
    return HMESH_ERROR;
  }

  /* Checking 2^N alignment */
  size_t n = new_nobj, reminder = 0;
  while(n) {
    if(reminder) {
      HmeshError("MemblockSizeReset() : aborted. "
        "Doesn't follow 2^N alignment");
      return HMESH_ERROR;
    }
    reminder = n & 1;
    n = n >> 1;
  } 
  HMESH_MEMBLOCK_SIZE = new_nobj;

  return HMESH_NO_ERROR; 
}

size_t MemblockSize() {
  return HMESH_MEMBLOCK_SIZE;
}

/* The number of blocks a pool can accomodate.
.. fixme : it should be changed. (rather) ,
.. sum of all poolsize should be capped*/
static const _Index HMESH_MEMPOOL_SIZE = 1024;

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
    .. alignment of number of indices per block. 
    .. So returning.
    */
    free(pool);
    HmeshError("Mempool() : Block size greater than 1MB"); 
    return NULL;
  }

  if(block_size % 8) {
    /* Redundant: this is automatically achieved as
    .. nobjects = 2^N is enforced.
    .. Also preferred : object_size = 2^N, which is 
    .. not strictly implemented,
    .. however it can be followed if you use
    .. structure of array (SoA) format for data-structure
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
  pool->address = NULL;
  pool->stack = 
    IndexStack( HMESH_MEMPOOL_SIZE, 8, &pool->address);
  /* The free blocks linked list */
  _FreeBlock * fb = (_FreeBlock *) address;
  fb->next = NULL;
  /* Encode this safety number to avoid double freeing */
  fb->safety = 0xFBC9183;
  fb->memblock = (_Memblock) {.pool = pool,.iblock = 0};
  pool->free_blocks = fb;

#ifndef _HMESH_VERBOSE_OFF
  fprintf(stdout, "\n1 x Pool Created with \n+1 x MemBlock "
    "[%ld Bytes = %ld x %ld Bytes]",
    pool->block_size, MemblockSize(), pool->object_size);
#endif

  return pool;
}

/* returns a memory block from the pool */
_Memblock MempoolAllocateFrom(_Mempool * pool) {
  /* Allocate a memory bloc  from the pool of blocks */

  if (!pool) {
    HmeshError("MempoolAllocateFrom() : No pool Mentioned");
    return (_Memblock) {.pool = NULL, .iblock = 0};
  }

  /* Pop a block index 'iblock' from index stack */
  _Index iblock = IndexStackFreeHead(&pool->stack, 1);
  if( iblock == UINT16_MAX ) { 
    HmeshError("MempoolAllocateFrom() : reached pool limit");
    return (_Memblock) {.pool = NULL, .iblock = 0};
  }

  /* Send a free Memblock {.pool, .iblock}, if any available 
  */
  if(pool->free_blocks) {
    _FreeBlock * fb = pool->free_blocks;
    if(fb->safety != 0xFBC9183) {
      HmeshError("MempoolAllocateFrom() : Double allocation"); 
      return (_Memblock) {.pool = NULL, .iblock = 0};
    }
    pool->address[iblock] = fb;
    fb->safety = 0xC1C2A9F;  
    pool->free_blocks = fb->next;
#ifndef _HMESH_VERBOSE_OFF
    fprintf(stdout, 
      "\n>1 x MemBlock [%ld Bytes = %ld x %ld Bytes]",
      pool->block_size, MemblockSize(), pool->object_size);
#endif
    return (_Memblock) {.pool = pool, .iblock = iblock};
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
  pool->address[iblock] = address;

#ifndef _HMESH_VERBOSE_OFF
  fprintf(stdout, 
    "\n+1 x MemBlock [%ld Bytes = %ld x %ld Bytes]",
    pool->block_size, MemblockSize(), pool->object_size);
#endif

  return (_Memblock) {.pool = pool, iblock = iblock};
}

/* Deallocate memory block back to the pool
*/
_Flag MempoolDeallocateTo(_Memblock memblock) {

  _Mempool * pool = (_Mempool *) memblock.pool;
  void * address = MemblockAddress(memblock);

  if ( !address ) {
    HmeshError("MempoolDeallocateTo() : block not mentioned");
    return HMESH_ERROR;
  }

  /* Add this block back to the free list of blocks */
  _FreeBlock * fb = (_FreeBlock*) address;
  fb->next = pool->free_blocks;
  if(fb->safety == 0xFBC9183) {
    HmeshError("MempoolDeallocateTo() : double freeing");
    return HMESH_ERROR;
  }
  fb->safety = 0xFBC9183;
  fb->memblock = memblock;
  pool->free_blocks = fb;

  /* Pop index out of used list */
  pool->address[memblock.iblock] = NULL;
  IndexStackDeallocate(&pool->stack, memblock.iblock);
#ifndef _HMESH_VERBOSE_OFF
  fprintf(stdout, 
    "\n<1 x MemBlock [%ld Bytes = %ld x %ld Bytes]",
    pool->block_size, MemblockSize(), pool->object_size);
#endif
  
  /* success */
  return HMESH_NO_ERROR;
}

/* Destroy pool*/
_Flag MempoolFree(_Mempool * pool) {
  if (!pool) { 
    HmeshError("MempoolFree() : pool not mentioned");
    fflush(stderr);
    return HMESH_ERROR;
  }

  /* Remove all the free blocks */
  _FreeBlock * fb = pool->free_blocks, * next = NULL;
  while(fb) {
    next = fb->next;
    free(fb);
    fb = next;
#ifndef _HMESH_VERBOSE_OFF
    fprintf(stdout, 
      "\n-1 x MemBlock [%ld Bytes = %ld x %ld Bytes]",
      pool->block_size, MemblockSize(), pool->object_size);
#endif
  }
  
  /* Forcefully remove blocks in-use */
  _Flag status = HMESH_NO_ERROR;
  _Index nuse = pool->stack.n;
  if(nuse) {
    HmeshError("MempoolFree() : Warning! "
      "%d blocks are still be in use", nuse);
    status |= HMESH_ERROR;
    _IndexStack * stack = &pool->stack;
    while(nuse--) {
      _Index iblock = stack->info[nuse].in_use;
      void ** address = pool->address + iblock;
      assert(*address);
      free(*address);
      *address = NULL;
      status |= IndexStackDeallocate(stack, iblock);
#ifndef _HMESH_VERBOSE_OFF
      fprintf(stdout, 
        "\n-1 x MemBlock [%ld Bytes = %ld x %ld Bytes]",
        pool->block_size, MemblockSize(), pool->object_size);
#endif
    }
  }

  status |= IndexStackDestroy(&pool->stack);
  free(pool->address);
  free(pool);
  
  return status;
}
