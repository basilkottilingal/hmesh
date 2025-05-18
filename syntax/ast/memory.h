#ifndef _H_AST_POOL_
#define _H_AST_POOL_

  /* 
  .. A block of size 1 MB (2^20) is allocated each time you run 
  .. out of memory. 
  ..
  .. Thread Safety : NOT THREAD SAFE
  */

  #ifndef _H_AST_BLOCK_SIZE_
    #define _H_AST_BLOCK_SIZE_ (1<<20)    /* 1 MB     */
  #endif

  typedef struct _AstPool {
    size_t size;
    void * fhead;
  } _AstPool ;

  /*
  .. Following are the api functions repsectively to
  .. (a) allocate a memory chunk of size 'size'.
  ..     NOTE : This is preferred to be used internally
  ..     for large chunks ~ page size ~ 1<<12 B.
  ..     'size' will be rounded off to next 8 Bytes
  .. (b) initialize a pool with nodes each of size 'size'
  ..     NOTE : 'size' SHOULD be >= 8.
  ..     RECOMMENDED : size % 8 == 0. 
  .. (c) allocate a node from pool
  .. (d) deallocate a node back to pool.
  ..     NOTE : Not recommended, especially in case of
  ..     AST construction, as nodes created (for ast nodes)
  ..     usually survive till the end
  .. (e) strdup() equivalent but memory is pooled by
  ..     pool handler in memory.c. Faster.
  .. (f) deallocate all memory blocks.
  */
  extern void *     ast_allocate_internal ( size_t size );
  extern _AstPool * ast_pool ( size_t size );
  extern void *     ast_allocate_from ( _AstPool * pool );
  extern void       ast_deallocate_to ( _AstPool *, void * node );
  extern char *     ast_strdup( const char * );
  extern void       ast_deallocate_all ();
  
#endif
