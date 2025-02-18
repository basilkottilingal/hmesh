#include <data-structure.h>
enum NODE_TYPE {
  NODE_IS_USED = 1,
  NODE_IS_RESERVED = 2
};

enum VARIABLE_TYPE {
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
_Node * NodeNew(_NodeBlock * block) {

  /* 'NodeNew(block)' : return an available node
  .. in the 'block', (if any)
  */

  /* In case there is no empty nodes available,
  .. return NULL*/ 
  if (!block->empty) 
    return NULL;

  _Node * node = block->empty;
  block->empty = node->next;

  /* NOTE: There should be one reserved node each at 
  .. left and right extremum of the block, otherwise 
  .. 'prev' or 'next' might by NULL
  */
  node->prev = block->used;
  node->next = block->used->next;
  
  /* 'block->used' does't change at any point */

  node->flags |= NODE_IS_USED;
#ifdef _MANIFOLD_DEBUG
  /* Redundant information. Maybe used only for debugging
  */
  --(block->nempty);
#endif
  
  /* return index, so you can do something with the index, 
  .. like setting scalar etc*/
  return node;  
}

static inline
_Flag NodeDestroy(_NodeBlock * block, _Node * node) {

  /* 'NodeDestroy(block, node)' : destroy an occuppied
  .. 'node' in the 'block'. Return success (1) if the
  .. 'node' is occuppied.
  */
  
  if(! (node->flags & NODE_IS_USED) )
    //error
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

  if(!block) {
    fprintf(stderr, 
      "Error : Couldn't create memory block\n");
    return NULL;
  }

  _NodeBlock * block = 
    (_NodeBlock *) malloc (sizeof(_NodeBlock));

  /* Set the field of te object block */
  block->address = address;
  block->start = (_Node *) (address + sizeof(_NodeBlock));
   
  /* Set nodes of the blocks as empty.
  .. NOTE: first and last nodes are not reserverd. 
  */
  _Node * first = (_Node *) address,
    * last = first + (block_size - 1);
  /* Used list. It's empty. 'first'/'last' are reserved 
  */
  block->used  = first;
  first->next = last;
  last->prev = first;
  first->prev = last->next = NULL;
  first->flags = last->flags = NODE_IS_RESERVED;

  /* Empty list. [1, block_size-2] are empty.
  */
  block->empty = first + 1; 
  for (size_t i = 1; i < block_size - 1; ++i) { 
    first[i].next = first + i + 1;
    /* first + i is neither reserved nor used */
    first[i].flags = 0; 
  }
  first[block_size - 2].next = NULL; 
}

Flag IndexBlockDestroy(_NodeBlock * block) {
  
}
