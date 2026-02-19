/*
.. TODO : Thread safety
.. (a) mutex lock would be ideal, as atomic insertion/deletion
.. might be too complex for a double linked list of free pool nodes.
*/

#ifndef _HMESH_TREE_POOL_
#define _HMESH_TREE_POOL_

#ifdef __cplusplus
extern "C" {
#endif

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
  
  
  /*
  .. Page size. Most modern compilers use 2^12 as page size
  */
  #define HMESH_PAGE_SIZE (1<<12)
  
  /*
  .. We restrict the max depth to 3. So all the common datatypes can be
  .. represented
  */
  #ifndef HMESH_TREE_POOL_DEPTH
  #define HMESH_TREE_POOL_DEPTH 3
  #endif
  
  /* 
  .. Linked list of free blocks.
  .. 'prev' & 'next' : to form a linked list of empty blocks.
  .. 'blockID' : Store information of itree, inode.
  */
  typedef struct FreeTBlock
  {
    struct FreeTBlock * prev, * next;
    Index blockID; 
  } FreeTBlock;
  
  /* 
  .. 'root' : address of each root node
  .. 'flags' : info on each nodes correspodning to memory blocks,
  .. Encodes depth, is a free/used node etc
  .. 'size' : size of root. For munmap()
  */
  typedef struct
  {
    void * root;
    uint8_t * flags;  
    size_t size;
  } HmeshTpool;
  
  /* 
  .. Allocate a block of 
  .. [HMESH_TREE_POOL_SIZE] X [2^d],
  .. where d = HMESH_TREE_POOL_SIZE - depth
  .. Allocate using a obj_size.
  .. It allocates obj_size * PAGE_SIZE
  .. Allocate A page
  .. Get the memory address from starting index
  .. Deallocate
  .. Free all. @ the end of pgm
  ..
  */
  extern Index  hmesh_tpool_allocate         ( int depth ); 
  extern Index  hmesh_tpool_allocate_general ( size_t obj_size ); 
  extern Index  hmesh_tpool_allocate_page    ( );
  extern void * hmesh_tpool_address          ( Index blockID );
  extern int    hmesh_tpool_deallocate       ( Index blockID );
  extern void   hmesh_tpool_destroy          ( );
  extern HmeshTpool * hmesh_tpool_tree       ( int itree );
  extern size_t hmesh_tpool_block_size       ( );

#ifdef __cplusplus
}
#endif
  
#endif
