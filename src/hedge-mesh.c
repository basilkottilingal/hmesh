/* Add a block @ iblock-th position.
.. Return memory address of the block, if successful
*/
static void * 
HmeshArrayAdd(_HmeshArray * a, _Index iblock) {

  _IndexStack * stack = &a->stack;
  if( StackIndexAllocate(stack, iblock) ) {
    HmeshError("HmeshArrayAdd() : iblock %d out of bound "
               "or already in use", iblock);
    return NULL;
  }

  /* get a block from pool */
  _Memblock b = _MempoolAllocateFrom(a->pool);
  void * m = MemblockAddress(b);
  if(!m) {
    HmeshError("HmeshArrayAdd() : memory pooling failed");
    StackIndexDeallocate(stack, iblock);
    return NULL;
  }
  
  /* expand address array if needed */
  if(a->stack.max > a->max) {
    a->max = a->stack.max;
    a->address = (void **) 
      realloc(a->address, a->max * sizeof(void *));
    a->iblock = (_Index *) 
      realloc(a->iblock, a->max * sizeof (_Index));
  }
  /* Starting address of block is stored for easy access */
  a->address[iblock] = m;
  /* This information is used for deallocation of this block */
  a->iblock[iblock] = b.iblock; 

  return m;
}

/* Make sure that attribute 'a' has same number of
.. blocks as that of the 'reference' array.
*/
static _Flag
HmeshArrayAccomodate(_HmeshArray * reference,
  _HmeshArray * a) 
{
  if(!(reference && a)) {
    HmeshError("HmeshArrayAccomodate() : aborted");
    return HMESH_ERROR;
  }

  _IndexStack * stack = &reference->stack;
  for(int i=0; i < stack->n; ++i) {
    _Index iblock = stack->info[i].in_use;
    if ( ( iblock < a->max) ? (!a->address[iblock]) : 1 )
      if( !HmeshArrayAdd(a, iblock) ) {
        HmeshError("HmeshArrayAccomodate() : "
                   "HmeshArrayAdd() : failed");
        return HMESH_ERROR;
      }
  }

  return HMESH_NO_ERROR;
}

/* deallocate an existing block @ iblock-th position 
.. Return HMESH_NO_ERROR if successful*/
static
_Flag 
HmeshArrayRemove(_HmeshArray * a, _Index iblock){

  void * address = ( iblock < a->max) ? 
      a->address[iblock] : NULL;

  if(!address) {
    HmeshError("HmeshArrayRemove() : "
               "cannot locate memory block");
    return HMESH_ERROR;
  }

  _Index status = MempoolDeallocateTo( (_Memblock) 
            {.pool = a->pool, .iblock = a->i[iblock]});
  if(!status) {
    a->address[iblock] = NULL;
    status |= IndexStackDeallocate(&a->stack, iblock);
  }

  if(status) HmeshError("HmeshArrayRemove() : "
               "Index or Memblock deallocation failed");

  return status;
}

/* Create a new attribute */
static _HmeshArray * 
HmeshArray(char * name, size_t size) {

  _Mempool * pool = Mempool(size);//MempoolGeneral(size);
  if(!pool) {
    HmeshError("HmeshArray() : aborted");
    return NULL;
  }

  /* A new attribute */
  _HmeshArray * a = 
    (_HmeshArray *)malloc(sizeof(_HmeshArray));
  a->pool    = pool;
  a->address = NULL;
  a>stack = IndexStack(HMESH_MAX_NBLOCKS, 4, &a->address);
  a->max  = a->stack.max;
  if(a->max) 
    a->iblock = (_Index *) malloc (a->max * sizeof(_Index));

  /* NOTE : For scalars 'name' is necessary */
  if(!name) { 
    HmeshError("HmeshArray() : "
               "Warning. attribute with no specified name");
    a->name[0] = '\0';
    return a;
  }

  /* Set attribute name */
  _Flag s = 0;
  char * c = name;
  /* length of name is restricted to 32 including '\0' */
  while(*c && s<31){
    /* fixme : make sure naming follows C naming rules */
    a->name[s++] = *c++;
  }
  a->name[s] = '\0';

  return a;
}

/* Destroy an attribute object. 
.. return HMESH_NO_ERROR if successful 
*/
_Flag 
HmeshArrayDestroy(_HmeshAttrribute * a) {

  _Flag status = HMESH_NO_ERROR;
  /* Remove all blocks in use */
  for(int i=0; i<a->max; ++i) 
    if(a->address[i]) {
      if(HmeshArrayRemove(a, i) != HMESH_NO_ERROR ) { 
        status = HMESH_ERROR;
        HmeshError("HmeshArrayDestroy() : "
                   "cannot free memblock");
      }
      if(IndexStackDeallocate(&a->stack, a->iblock[iblock])
          != HMESH_NO_ERROR) { 
        status = HMESH_ERROR;
        HmeshError("HmeshArrayDestroy() : "
                   "cannot free block index");
      }

      /* In case of any unsuccessful freeing, you may
      .. expect memory related "unexpected behaviour"*/
      a->address[i] = NULL;
    }
  
  /* NOTE: since pool is a general pool for all attributes
  .. of same object_size, it will not be freed here. 
  */
  if(address)     
    free(address);
  if( IndexStackDestroy(&a->stack) != HMESH_NO_ERROR ){
    HmeshError("HmeshArrayDestroy() : "
               "index stack cannot be cleaned. "
               "(Memory Leak) ");
  }
  MempoolDestroy(a->pool);
  free(a);

  return status;
}

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

/* return a newly allocated node in the block. 
.. In case block if fully occupied, return 0.
.. '0' is an invalid (or reserved) node.*/
static inline
_Index HmeshNodeNew(_HmeshNode node) {

  /* In case there is no empty nodes available,
  .. return */
  _IndexStack * stack = &node->prev.stack;
  _Index index = IndexStackFreeHead(stack, 0);
   
  for(int i = 0; i < stack->nfree; ++i) {
    _Index index = stack->info[i].in_use;
  }
  if (!block->empty) 
    return 0;

  _HmeshNode * node = block->empty;
  block->empty = node->next;

  /* NOTE: There should be one reserved node each at 
  .. left and right extremum of the block, otherwise 
  .. 'prev' or 'next' might by NULL
  */
  node->prev = block->used->prev;
  node->next = block->used;
  
  /* return index, so you can do something with the index, 
  .. like setting scalar etc*/
  return node;  
}

/* Deallocate a node back to block.
.. TODO: add a safety check to avoid double freeing */
static inline _Flag 
HmeshNodeDestroy(_HmeshNodeBlock * block, _HmeshNode * node) {
   
  node->next->prev = node->prev;
  node->prev->next = node->next;

  node->next = block->empty;
  block->empty = index;

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
