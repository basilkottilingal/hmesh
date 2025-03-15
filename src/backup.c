

/* add a new scalar 'name' to the cells 'c' 
*/
_Flag
_HmeshCellsAddScalar(_HmeshCells * c, char * name) {

  /* NOTE : scalars need to have a name */
  size_t len = name ? strlen(name) : 0 ;  
  if( (!len) || (len > HMESH_MAX_VARNAME) ) { 
    HmeshError("HmeshCellsAddScalar() : "
               "invalid scalar name");
    return HMESH_ERROR;
  }

  void ** scalars = c->scalars;
  _IndexStack * stack = &c->iscalar;
  for(int i=0; i < stack->n; ++i) {
    _Index index = stack->info[i].in_use;
    _HmeshArray * s = (_HmeshArray *) (scalars[index]);
    if(!s) {
      HmeshError("HmeshCellsAddScalar() : "
                 "expected a scalar");
      continue;
    } 
    if( !strcmp( name, s->name) ) {
      HmeshError("HmeshCellsAddScalar() : scalar with "
                 "name '%s' exists", name);
      return HMESH_ERROR;
    }
  }

  _Index index = IndexStackFreeHead(stack, 1);
  if(index == UINT16_MAX) { 
    HmeshError("HmeshCellsAddScalar() : "
               "ran out of scalar index");
    return HMESH_ERROR;
  }

  /* create a new scalar with name */
  _HmeshArray * s = 
    (_HmeshScalar *) HmeshArray(name, sizeof(_Real));
  if(!s) {
    HmeshError("HmeshCellsAddScalar() : "
               "scalar initialization failed");
    return HMESH_ERROR;
  }
  scalars[index] = s;

  /* make sure attribute has same number of blocks as nodes */
  HmeshArrayAccomodate(c->nodes.prev, s);

  return HMESH_NO_ERROR;
}

/* remove scalar 'name' from the cells 'c' 
*/
_Flag
_HmeshCellsRemoveScalar(_HmeshCells * c, char * name) {

  size_t len = name ? strlen(name) : 0;  
  if( (!len) || (len > HMESH_MAX_VARNAME) || 
      (c->iscalars.n == 0) ){
    HmeshError("HmeshCellsRemoveScalar() : aborted()");
    return HMESH_ERROR;
  }

  void ** scalars = c->scalars;
  _IndexStack * stack = &c->iscalar;
  for(int i=0; i < stack->n; ++i) {
    _Index index = stack->info[i].in_use;
    _HmeshArray * s = (_HmeshArray *) (scalars[index]);
    if(!s) {
      HmeshError("HmeshCellsAddScalar() : "
                 "expected a scalar");
      continue;
    } 
    if( !strcmp( name, s->name) ) {
      if( HmeshArrayDestroy(s)) {
        HmeshError("HmeshCellsRemoveScalar() : "
                   "HmeshArrayDestroy(%s) failed", name);
        return HMESH_ERROR;
      } 
      if( IndexStackDeallocate(stack, index) ) {
        HmeshError("HmeshCellsRemoveScalar() : "
                   "index cannot be removed");
        return HMESH_ERROR;
      }
      return HMESH_NO_ERROR;
    }
  }

  HmeshError("HmeshCellsRemoveScalar() : "
             "scalar '%s' doesn't exist", name);
  return HMESH_ERROR;
}

static inline
_Index HmeshNodeGetBlock(_HmeshNode * nodes ) { 
  /* In case there is no empty nodes available,
  .. return */
  _Index iblock = IndexStackFreeHead(&nodes->stack, 0);

  if(index > nodes->max) {
    /* Creating a new block if needed */
    node->info = (_Index *)
      realloc(nodes->info, 4*nodes->stack.max, sizeof(_Index));
    if( HmeshArrayAdd(nodes->prev, iblock) ||
        HmeshArrayAdd(nodes->next, iblock) ) {
      HmeshError("HmeshNodeNew() : failed");
      return (_Node){.index = UINT16_MAX, iblock = UINT16_MAX};
    }
    _Index * prev = (_Index *) nodes->prev->address[iblock],
      * next = (_Index *) nodes->prev->address[iblock],
      * info = nodes->info + (4*iblock),
      bsize = (_Index) MemblockSize();

    for(_Index index = 1; index < bsize-1; ++index) 
      next[index] = index + 1;
    prev[0] = next[0] = next[bsize-1] = 0;

    /* head of in-use and free-list linked list respectively*/
    info[0] = 0;
    info[1] = 1;
    /* number of nodes in-use and free */ 
    info[2] = 0;
    info[3] = bsize - 1; 
  }
}

/* return a newly allocated node in the block. 
.. In case block if fully occupied, return 0.
.. '0' is an invalid (or reserved) node.*/
static inline
_Index HmeshNodeNew(_HmeshNode * nodes) {

  _Index iblock = HmeshNodeGetBlock(nodes);
    
  _Index * prev = (_Index *) nodes->prev->address[iblock],
    * next = (_Index *) nodes->prev->address[iblock],
    * info = nodes->info + (4*iblock), index = info[1], 
    head = info[0];
  /* Update head of free list */
  info[1] = next[index];
  /* adding 'index' to used linked list */
  next[index] = head;
  prev[index] = prev[head];
  next[prev[head]] = prev[head] = index;
  info[0] = index;
  /* Count on used/free indices */
  info[2]++;
  info[3]--;
  
  /* If node block is full, */
  if(!info[0] ) {
    //IndexStackAllocate(stack, iblock);
  }
  
  /* return Node */
  return (_Node) {.index = index, .iblock = iblock};  
}

/* Deallocate a node back to block.
.. TODO: add a safety check to avoid double freeing */
static inline _Flag 
HmeshNodeDestroy(_HmeshNode * nodes, _Node node) {

  _Index 
    * prev = (_Index *) nodes->prev->address[node.iblock],
    * next = (_Index *) nodes->next->address[node.iblock],
    index = node.index, * info = nodes->info + (4*index);

  prev[next[index]] = prev[index];
  next[prev[index]] = next[index];

  next[index] = info[1];
  info[1] = index;

  return 1;  
}

/* Create a new block */

_HmeshNodeBlock * 
HmeshNodeBlockNew(_HmeshArray * a, _Flag iblock) {

  void * address = a->address[iblock];

  if(!address) {
    HmeshError("HmeshNodeBlockNew() : aborted()");
    return NULL;
  }

  _HmeshNodeBlock * block = 
    (_HmeshNodeBlock *) malloc (sizeof(_NodeBlock));

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

#define HNODE ( (_Node) \
  {.iblock = iblock, .index = index} )

static inline 
_Node HmeshNode(_HmeshNode * nodes) {

  _Flag iblock = nodes->free_blocks[0];
  if( !iblock ) 
    return (_Node) {.iblock = 0, .index = 0};
  
  _Index * prev = (_Index *) nodes->prev->address[iblock],
    * next = (_Index *) nodes->next->address[iblock];

  _Index index = nodes->free_node[iblock],
         head = nodes->head_node[iblock];

  nodes->head_node[iblock] = next[index];
  if(nodes->head_node[iblock]) {
    /* create a new block */
  }
 
  next[index] = next[head];
  prev[index] = head;
  next[head] = prev[next[index]] = index;

  return HNODE;
}

static inline 
_Flag HmeshNode(_HmeshNode * nodes, _Node node) {

  _Flag iblock = nodes->free_blocks[0];
  if( !iblock ) 
    return (_Node) {.iblock = 0, .index = 0};
  
  _Index * prev = (_Index *) nodes->prev->address[iblock],
         * next = (_Index *) nodes->next->address[iblock];

  _Index index = nodes->free_node[iblock],
         head = nodes->head_node[iblock];

  nodes->head_node[iblock] = next[index];
  if(nodes->head_node[iblock]) {
    /* create a new block */
  }
 
  next[index] = next[head];
  prev[index] = head;
  next[head] = prev[next[index]] = index;

  return (_Node) {.iblock = iblock, .index = index};
}
