/* for array 'a' (or scalar array 's'), 
.. iblock 'ib' in [0, HMESH_MAX_NBLOCKS) and .. 
.. index 'index' in [0,HMESH_BLOCK_SIZE) */
#define HINDEX(a,ib,index) \
  ( ((_Index) a->address[ib])[index] )
#define HREAL(s,ib,index) \
  ( ((_Real) s->address[ib])[index] )
#define HFLAG(a,ib,index) \
  ( ((_Flag) a->address[ib])[index] )
#define HIBLOCK(a,ib,index) HFLAG(a,ib,index)
#define HPID(a,ib,index) \
  ( ((int) a->address[ib])[index] )
#define HMAP_REAL(s,indices,blocks,ib,index) \
  (HREAL(s, HIBLOCK(blocks,ib,index), HINDEX(indices,ib,index)))

typedef struct {
  /* indices [1,max) are either in use/free .
  .. Index 0 is reserved and used in place of NULL.
  .. This is only for small 'max' (say <= 64). 
  */

  _Index * in_use, * free_list, * loc,
           limit, n, nfree, max;
} _IndexStack;
static inline _IndexStack IndexStack(_Index limit) {
  /* Note : There is no error checking */
  _Index * in_use = (_Index *) malloc (4* sizeof(_Index)), 
    * free_list = (_Index *) malloc (4* sizeof(_Index)),
    * loc = (_Index *) malloc (4* sizeof(_Index));
  free_list[0] = 0;
  free_list[1] = 1;
  free_list[2] = 2;
  free_list[3] = 3;
  loc[0] = loc[1] = loc[2] = loc[3] = UINT16_MAX;
  return (_IndexStack) 
    {.in_use = in_use, .free_list = free_list, .loc = loc, 
     .limit = limit, .n = 0, .nfree = 4, .max = 4} ;
}

static inline 
_Index IndexStackNew(_IndexStack * stack, int isPop) {
  if(!stack->nfree) {
    if(stack->max == stack->limit)  
      /* Reached limit. Return invalid index*/
      return UINT16_MAX;
    /* Expand the array */
    _Index max = ((stack->max + 4) > stack->limit) ? 
      stack->limit : (stack->max + 4);
    stack->in_use = (_Index **) 
      realloc (stack->in_use, max * sizeof(_Index));
    stack->free_list = (_Index **) 
      realloc (stack->free_list, max * sizeof(_Index));
    stack->loc = (_Index **) 
      realloc (stack->loc, max * sizeof(_Index));
    for(int i = stack->max; i<max; ++i) {
      stack->free_list[i-stack->max] = i;
      stack->loc[i] = UINT16_MAX;
    }
    stack->nfree = max - stack->max;
    stack->max = max;
  }
  _Index index = stack->free[stack->nfree-1];
  if(isPop) { 
    stack->nfree--;
    stack->in_use[stack->n] =  index;
    stack->loc[index] = stack->n++;
  }
  return index;
}

static inline 
_Flag IndexStackFree(_IndexStack * stack, _Index index) {
  /* index location in in_use array */
  _Index indexloc = 
    (index >= stack->max) ? UINT16_MAX : stack->loc[index];
  if( indexLoc == UINT16_MAX ) {
    /* This index is not an in_use index */
    return HMESH_ERROR;
  }
  stack->free_list[stack->nfree++] = index; 
  stack->in_use[indexLoc] = stack->in_use[--(stack->n)];
  stack->loc[index] =  UINT16_MAX;

  return HMESH_NO_ERROR;
}

/* Add a block @ iblock-th position.
.. Return memory address of the block, if successful
*/
static 
void * 
HmeshArrayAdd(_HmeshArray * a, _Flag iblock) {

  /* handling unexpected behaviour */
  if ( (!iblock ) || (iblock > HMESH_MAX_NBLOCKS) ) {
    HmeshError("HmeshArrayAdd() : iblock out of bound");
    return NULL;
  }
  if(iblock < a->max) {
    /* In case a block already exist @ this location */
    if(address[iblock]) {
      HmeshError("HmeshArrayAdd() : iblock in use");
      return NULL;
    }
  }
  else {
    a->address = (void **) 
      realloc (a->address, (1 + iblock) * sizeof(void *));
    for(int i = a->max; i <= iblock; ++i)
      a->address[i] = NULL;
    a->max = 1 + iblock;
  }

  /* get a block from pool */
  _Memblock b = _MempoolAllocateFrom(a->pool);
  void * m = MemblockAddress(b);
  if(!m) {
    HmeshError("HmeshArrayAdd() : memory pooling failed");
    return NULL;
  }

  /* This information is used for deallocation of this block */
  a->i[iblock] = b.iblock;
  /* Starting address of block is stored for easy access */
  a->address[iblock] = m;

  return m;
}

/* Make sure that attribute 'a' has same number of
.. blocks as that of the 'reference' array.
*/
static 
_Flag
HmeshArrayAccomodate(_HmeshArray * reference,
  _HmeshArray * a) 
{
  if(!(refeence && a)) {
    HmeshError("HmeshArrayAccomodate() : aborted");
    return HMESH_ERROR;
  }

  for(int i=0; i < reference->max; ++i) {
    _Flag yes = (i < a->max) ? (a->address[i] != NULL) : 1; 
    if ( yes )
      if( !HmeshArrayAdd(a, i) ) {
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
HmeshArrayRemove(_HmeshArray * a, _Flag iblock){

  _Flag status = 
    iblock < a->max ? (address[iblock] != NULL) : 0;

  if(!status) {
    HmeshError("HmeshArrayRemove() : "
               "cannot locate memory block");
    return HMESH_ERROR;
  }

  status = MempoolDeallocateTo( (_Memblock) 
            {.pool = a->pool, .iblock = a->i[iblock]});

  if(status == HMESH_NO_ERROR)  
    a->address[iblock] = NULL;
  else 
    HmeshError("HmeshArrayRemove() : "
               "MempoolDeallocateTo() : failed");

  return status;
}

/* Create a new attribute */
_HmeshArray * 
HmeshArray(char * name, size_t size) {

  _Mempool * pool = MempoolGeneral(size);
  if(!pool) {
    HmeshError("HmeshArray() : aborted");
    return NULL;
  }

  /* A new attribute */
  _HmeshArray * a = 
    (_HmeshArray *)malloc(sizeof(_HmeshArray));
  a->pool = pool;
  a->address = (void **)malloc(2*sizeof(void *));
  a->address[0] = a->address[1] = NULL;
  a->max = 2;
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
                   "cannot free block");
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
  free(a);

  return status;
}

/* add a new scalar 'name' to the cells 'c' 
*/
_Flag
_HmeshCellsAddScalar(_HmeshCells * c, char * name) {

  /*
  if(!c->scalars) {
    _HmeshScalar * scalars = 
      (_HmeshScalar *) malloc (sizeof(_HmeshScalar));
    _HmeshArray ** s = 
      (_HmeshArray **) malloc (10*sizeof(_HmeshScalar *));
    if(! (s && scalars))
  }
  */
 
  _HmeshScalar * scalars = c->scalars;

  /* NOTE : scalars need to have a name */
  size_t len = name ? strlen(name) : 0 ;  
  if( (!len) || (len > HMESH_MAX_VARNAME) || 
      (c->scalars ? (c->scalars->max==HMESH_MAX_NVARS) : 1) ){
    HmeshError("HmeshCellsAddScalar() : aborted()");
    return HMESH_ERROR;
  }

  for(int i=0; i < c->max; ++i) {
    _HmeshArray * s = c->s[i];
    if(!s) continue;
    if( !strcmp( name, s->name) ) {
      HmeshError("HmeshCellsAddScalar() : scalar with name "
                 "'%s' exists", name);
      return HMESH_ERROR;
  }
  _HmeshArray * s = (_HmeshScalar *) 
    HmeshArray(name, sizeof(_DataType), HMESH_SCALAR_POOL);
  if(!s) return HMESH_ERROR;
  
  c->nscalars += 1;
  c->s = (_HmeshScalar **) 
    realloc((c->nscalars)*sizeof(_HmeshScalar *));
  c->s[c->nscalars - 1] = s;

  /* make sure attribute has same number of blocks as nodes */
  HmeshArrayAccomodate(c->nodes, s);

  return HMESH_NO_ERROR;
}

/* remove scalar 'name' from the cells 'c' 
*/
_Flag
_HmeshCellsRemoveScalar(_HmeshCells * c, char * name) {

  size_t len = name ? strlen(name) : 0;  
  if( (!len) || (len > HMESH_MAX_VARNAME) || 
      (c->nscalars == 0) ){
    HmeshError("HmeshCellsRemoveScalar() : aborted()");
    return HMESH_ERROR;
  }

  for(int i=0; i < c->nscalars; ++i) {
    _HmeshScalar * s = c->s[i];
    if(!s) continue;
    if( !strcmp( name, s->name) ) {
      /* found the scalar */
      if( HmeshArrayDestroy((_HmeshArray *)s) !=
          HMESH_NO_ERROR ) {
        HmeshError("HmeshCellsRemoveScalar() : remove failed");
        return HMESH_ERROR;
      }
      c->s[i] = c->s[c->nscalars - 1];
      c->nscalars -= 1;
      c->s = (_HmeshScalar **) 
        realloc (c->nscalars * sizeof(_HmeshScalar *));
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
  for(int iblock = 0; iblock < node.prev->n; ++iblock) {
    
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
