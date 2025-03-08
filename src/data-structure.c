#include <data-structure.h>
enum HMESH_BITS {
  /* Info on data nodes */
  NODE_IS_USED = 1,
  NODE_IS_RESERVED = 2
  /* scalar or vector or tensor*/
  VAR_TENSOR_RANK = 1|2,
  /* rank of tensor = (VARIABLE_TYPE & VAR_TENSOR_RANK) */
  VAR_IS_A_SCALAR = 0,
  VAR_IS_A_VECTOR = 1,
  VAR_IS_A_TENSOR = 2,
  /* constant or variable */
  VAR_IS_A_CONSTANT = 4,
  /* corresponds to a vertex or a face */
  CELL_SHIFT = 3,
  CELL_LOCATION = (2|1) << CELL_SHIFT,
  /* (VARIABLE_TYPE & VAR_LOCATION) */
  CELL_VERTEX = 0,
  CELL_EDGE   = 1<<CELL_SHIFT,
  CELL_FACE   = 2<<CELL_SHIFT,
  CELL_VOLUME = 3<<CELL_SHIFT
};


static inline
_Flag NodeDestroy(_NodeBlock * block, _Node * node) {

  /* 'NodeDestroy(block, node)' : destroy an occuppied
  .. 'node' in the 'block'. Return success (1) if the
  .. 'node' is occuppied.
  */
#ifdef _MANIFOLD_DEBUG
  if( ! (block && node) )
    return 0;
  
  if( node->flags & NODE_IS_RESERVED )
    return 0;
#endif
  
  if(! (node->flags & NODE_IS_USED) )
    return 0;
   
  node->next->prev = node->prev;
  node->prev->next = node->next;

  node->next  = block->empty;
  block->empty = index;

  node->flags &= ~NODE_IS_USED;
#ifdef _MANIFOLD_DEBUG
  ++(block->nempty);
#endif

  //successfully removed index out of 'used' list
  return 1;  
}

_NodeBlock * NodeBlockNew(_Mempool * pool) {
  /* Look for memory of 2^15 nodes in pool or create a new 
  .. pool and add to the linked list of pool*/
  _Memblock memblock = MempoolAllocateFrom(pool);

  void * address = Memblock(memblock);

  if(!address) {
    fprintf(stderr, 
      "Error : Couldn't create memory block for nodes\n");
    return NULL;
  }

  _NodeBlock * block = 
    (_NodeBlock *) malloc (sizeof(_NodeBlock));

  /* Set the field of te object block */
  block->memblock = memblock;

  /* Number of nodes in the block */
  size_t n = pool->block_size/pool->object_size;

  /* Starting index of first node.
  .. Used for indexing of nodes. 
  */
  _Index i0 = n*memblock.iblock;
   
  /* Set nodes of the blocks as empty.
  .. NOTE: first and last nodes are not reserverd. 
  */
  _Node * first = (_Node *) address,
    * last = first + (n - 1);
  /* Reserved nodes */
  first->flags = NODE_IS_RESRVED;
  last->flags = NODE_IS_RESERVED;
  /* index */
  first->i = i0;
  last->i = i0 + n;
  /* Doubly linked list */
  first->next = last;
  first->prev = NULL; 
  last->prev = first;
  last->next = NULL;

  /* Used list. It's empty as of now.
  */
  block->used  = last;

  /* Empty list. Nodes [1, block_size-2] are empty.
  */
  block->empty = first + 1; 
  for (size_t i = 1; i < n - 1; ++i) { 
    /* next node in empty list */
    first[i].next = first + i + 1;
    /* first + i is neither reserved nor used */
    first[i].flags = 0; 
    /* index of this node */
    first[i].i = i0 + i;
  }
  first[block_size - 2].next = NULL; 

  /* Object functions to add/remove a node to this block
  */
  block->add = NodeNew;
  block->remove = NodeDestroy;

  return block;
}

Flag NodeBlockDestroy(_NodeBlock * block) {
  /* Destroy an Node block that stores indices */
  _Memblock memblock = block->memblock;
  void * address = Memblock(memblock) 
    
  if(!address) {
    fprintf(stderr, "\nError : Cannot delete this node block");
    fflush(stderr);
    return 0;
  } 

  /* NOTE : This memory block is deallocated back to the unused
  .. part of the pool. Don't re-use the address again.
  .. Also don't Deallocate again. There is no
  .. provision to know multiple de-allocation to the pool
  */
  MempoolDeallocateTo(memblock);

  return 1;
}
