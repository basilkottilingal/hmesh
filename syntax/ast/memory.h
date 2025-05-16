#ifndef _H_AST_POOL_
#define _H_AST_POOL_

  /* 
  .. 
  .. The memory pool is an arena allocator, where you cannot deallocate.
  .. Each allocated node survive till the end of the pgm.
  .. You can only delete the entire block(s) which you should do
  .. only at the end of the program.
  ..
  .. A block of size 1 MB (2^20) is allocated each time you run 
  .. out of memory. 
  ..
  .. Thread Safety : NOT THREAD SAFE
  */

  #ifndef _H_AST_BLOCK_SIZE_
    #define _H_AST_BLOCK_SIZE_ (1<<20)    /* 1 MB     */
  #endif


  /*
  .. Following are the api functions repsectively to
  .. (a) allocate a memory chunk of size 'size'.
  ..     NOTE : don't use this to allocate large chunks,
  ..     say, > 64 Bytes. (However, Limit is set as 4096 B).
  .. (b) deallocate all memory blocks.
  */
  extern void * ast_allocate(size_t);
  extern void ast_deallocate_all();
  
#endif
