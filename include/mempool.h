#include <stdio.h>
#include <stdlib.h>

#define MEMPOOL 

/** 
.. linked list of free node
.*/
typedef struct _FreeNode {
  _FreeNode * next; 
} _FreeNode;

/** 
 * linked list of memory blocks
 */
typedef struct 
_Mempool{

  /* Size of each object. 
  .. NOTE : Need to be 64 bit aligned datatype.  
  */ 
  size_t object_size; 
   
  /* Number of objects in the block.
  */
  size_t block_size; 

  /* list of pointers to the memory blocks. 
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
  _FreeNode * free_list; 
 
  /* No of freenodes in the block.
  .. NOTE: It should be, nfree <= nblocks < 256
  */
  _Flag nfree;   

} _Mempool;

_Mempool * Mempool(size_t object_size, size_t nobjects) {
  // Initialize the memory pool
  // FIXME: Redo this:

  if(object_size < sizeof (void *)){
    fprintf(stderr, 
      "\nError : Oject size should be atleast %ld ", 
      sizeof(void *));
    /* Otherwise you might overwrite next nodes ..
    .. when typecasting empty memspaces to _FreeNode * */
    fflush(stderr);  
    return NULL;
  }
  if(object_size % 8) {
    fprintf(stderr, 
      "\nError : Prefer a multiple of 64 bit for object size. ");
    fflush(stderr);  
    return NULL;
  }
    
  _Mempool * pool = 
    (_Mempool*)malloc(sizeof(_Mempool));
  if (!pool) {
    fprintf(stderr, "\nERROR: \
Failed to allocate memory for the pool.\n");
    fflush(stderr);  
    return NULL;
  }

  size_t Bytes = nobjects * object_size,
    kB = Bytes/1024, MB = Bytes/(1024*1024);
  
  //Rounding to closer kB , or MB
  size_t block_size = kB < 1024 ? //less than an MB?
    (kB+1)*1024 :  1024*1024; //1 MB is the limit
  
  size_t nfree = block_size/object_size;
  
  if(MB > 1) 
    //1MB is the limit
    fprintf(stderr, "\nWARNING: Block size rounded to 1MB");

  pool->object_size  = object_size;
  pool->block_size   = block_size;
  pool->nfree        = nfree;
  pool->memory_block = malloc(block_size);
  if (!pool->memory_block) {
    fprintf(stderr, "\nERROR:\
Failed to allocate memory block for the pool.");
    free(pool);
    return NULL;
  }

  // Initialize the free list
  pool->free_list = NULL;
  char * block = (char *)  pool->memory_block;
  for (size_t i = 0; i < nfree; i++,block+= object_size) {
    _FreeNode * node = (_FreeNode*) block;
    node->next = pool->free_list;
    pool->free_list = node;
  }

  fprintf(stdout, "\nMemBlock[%ld bytes] created. \n\
Can accomodate %ld Objects each of size %ld",
pool->block_size, pool->nfree, pool->object_size);
  fflush(stdout);

  return pool;
}

// Allocate memory from the pool
void * MempoolAllocateFrom(_Mempool * pool) {
  if (!pool){
    fprintf(stderr, "ERROR: No pool Mentioned!");
    fflush(stderr);
    return NULL;
  }

  if (!pool->free_list) {
    fprintf(stderr, "WARNING:\
No free slots available in the pool.");
    fflush(stderr);
    return NULL;
  }

  // Remove the first node from the free list
  _FreeNode *allocated_node = pool->free_list;
  pool->free_list = allocated_node->next;
  --(pool->nfree);

  return (void*)allocated_node;
}

// Deallocate memory back to the pool
void MempoolDeallocateTo(_Mempool * pool, void * ptr) {
  if (!pool || !ptr) {
    fprintf(stderr, "ERROR:\
Either of pool or node NOT mentioned!");
    fflush(stderr);
    return;
  }

  // Add the slot back to the free list
  _FreeNode *node = (_FreeNode*)ptr;
  node->next = pool->free_list;
  pool->free_list = node;
  ++(pool->nfree);
}

// Free the entire memory pool
void MempoolFree(_Mempool *pool) {
  if (!pool) return;

  free(pool->memory_block);
  free(pool);
}
