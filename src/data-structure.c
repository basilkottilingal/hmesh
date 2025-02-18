#include <data-structure.h>

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
_Index * IndexNew(_IndexBlock * block, _Flag d) {
  /* create new node to this block and return the index*/
  _Index * index = block->empty;
  if (!index) {
#ifdef _MANIFOLD_DEBUG
    assert(!block->nempty);
    fprintf(stderr, "Error: Fully occuppied block\n");
#endif
    return NULL;
  }
  block->empty = index->next;

  /* NOTE: There should be one reserved node each at 
  .. left and right extremum of the block, otherwise 
  .. 'prev' or 'next' might by NULL
  */
  index->prev = block->used;
  index->next = block->used->next;
  block->used = index;

  index->flags |= NODE_IS_USED;
#ifdef _MANIFOLD_DEBUG
  /* CELL_LOCATION or CELL_TYPE (vertex,edge,face or volume)
  */
  index->flags |= d << CELL_SHIFT;
  --(block->nempty);
#endif
  
  /* return index, so you can do something with the index, 
  .. like setting scalar etc*/
  return index;  
}

static inline
_Flag IndexDestroy(_IndexBlock * block, _Index * index) {
  
  if(! (index->flags & NODE_IS_USED) )
    //error
    return 0;
   
  index->next->prev = index->prev;
  index->prev->next = index->next;

  index->next  = block->empty;
  block->empty = index;

  index->flags &= ~NODE_IS_USED;
#ifdef _MANIFOLD_DEBUG
  ++(block->nempty);
#endif

  //successfully removed index out of 'used' list
  return 1;  
}

typedef struct {
  _Mempool * pool;
  void * address;
}_Memblock;

_IndexBlock * IndexBlockNew(_Mempool * pool) {
  /* Look for memory of 2^15 nodes in pool or create a new 
  .. pool and add to the linked list of pool*/
  _Memblock block = 
    MempoolAllocate(pool, block_size * sizeof(_IndexBlock));

  if(!block) {
    fprintf(stderr, 
      "Error : Couldn't create memory block\n");
    return NULL;
  }

  /* --------------------------|
    | struct _Mempool          |
    |                          |
    |--------------------------|
    | struct _IndexBlock       |
    |                          |
    |--------------------------|
    | _Index 0                 |
    |                          |
    | _Index 1                 |
    |                          |
    |   ...                    |
    |--------------------------|
  */


  _IndexBlock * block = 
    (_IndexBlock *) malloc (sizeof(_IndexBlock));

  //FIXME : pool might be different;
  block->pool = pool;
  block->address = address;
  block->start = (_Index *) (address + sizeof(_IndexBlock));
   
  /* Set nodes of the blocks as empty.
  .. NOTE: first and last nodes are not reserverd. 
  */
  _Index * first = (_Index *) address,
    * last = first + (block_size - 1);
  /* Used list. It's empty. 'first'/'last' are reserved 
  */
  block->used  = first;
  first->next = last;
  last->prev = first;

  /* Empty list. [1, block_size-2] are empty.
  */
  block->empty = first + 1; 
  for (size_t i = 1; i < block_size - 2; ++i) 
    first[i].next = first + i + 1;
  first[block_size - 2].next = NULL; 
}

Flag IndexBlockDestroy(_IndexBlock * block) {
  
}
