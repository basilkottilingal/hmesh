#include <common.h>
#include <mempool.h>
#include <hmesh.h>
#include <ctype.h>

/* Add a block @ iblock-th position.
.. Return memory address of the block, if successful
static 
*/
void * 
HmeshArrayAdd(_HmeshArray * a, _Index iblock) {

  _IndexStack * stack = &a->stack;
  if( IndexStackAllocate(stack, iblock) ) {
    HmeshError("HmeshArrayAdd() : iblock %d out of bound "
               "or already in use", iblock);
    return NULL;
  }

  /* get a block from pool */
  _Memblock b = a->pool ? MempoolAllocateFrom(a->pool) :
    MempoolAllocateGeneral(a->obj_size);
  if(!a->pool)
    a->pool = (_Mempool *) b.pool;
  void * m = MemblockAddress(b);
  if(!m) {
    HmeshError("HmeshArrayAdd() : memory pooling failed");
    IndexStackDeallocate(stack, iblock);
    return NULL;
  }
  
  if(a->stack.max > a->max) {
    a->max = a->stack.max;
    a->iblock = (_Index *) 
      realloc(a->iblock, a->max * sizeof (_Index));
  }
  /* Starting address of block is stored for easy access */
  a->address[iblock] = m;
  /* This information is used for deallocation of this block */
  a->iblock[iblock] = b.iblock; 

  return m;
}

/* deallocate an existing block @ iblock-th position 
.. Return HMESH_NO_ERROR if successful
static
*/
_Flag 
HmeshArrayRemove(_HmeshArray * a, _Index iblock){

  void * address = ( iblock < a->max) ? 
      a->address[iblock] : NULL;

  if(!address) {
    HmeshError("HmeshArrayRemove() : cannot locate memblock");
    return HMESH_ERROR;
  }

  _Index status = MempoolDeallocateTo( (_Memblock) 
    {.pool = a->pool, .iblock = a->iblock[iblock]});

  if(!status) {
    a->address[iblock] = NULL;
    status |= IndexStackDeallocate(&a->stack, iblock);
  }

  if(status) 
    HmeshError("HmeshArrayRemove() : deallocation error");

  return status;
}

/* Create a new attribute */
_HmeshArray * 
HmeshArray(char * name, size_t size) {
  /* returns  new attribute. No blocks allocated yet.*/

  _HmeshArray * a = 
    (_HmeshArray *)malloc(sizeof(_HmeshArray));
  a->obj_size = size;
  a->pool = NULL;
  a->address = NULL;
  a->stack = IndexStack(HMESH_MAX_NBLOCKS, 4, &a->address);
  a->max  = a->stack.max;
  if(a->max) 
    a->iblock = (_Index *) malloc (a->max * sizeof(_Index));

  if(!name ? 1 : !name[0]) { 
    /* if "name" of attribute is empty.
    .. NOTE : For scalars 'name' is necessary */
    HmeshError("HmeshArray() : Warning! No name specified");
    a->name[0] = '\0';
  } 
  else {
    /* Set attribute name. 
    .. Length of name is restricted to 32 including '\0' 
    .. fixme : make sure naming follows C naming rules */
    _Flag s = 0;
    char * c = name;
    while(*c && s<31)
      a->name[s++] = *c++;
    a->name[s] = '\0';
  }

  return a;
}

/* Destroy an attribute object. 
.. return HMESH_NO_ERROR if successful 
*/
_Flag 
HmeshArrayDestroy(_HmeshArray * a) {
  if(!a)
    return HMESH_ERROR;

  _Flag status = HMESH_NO_ERROR;
  /* Remove all blocks in use */
  for(int iblock = 0; iblock < a->max; ++iblock) 
    if(a->address[iblock]) {
      if( HmeshArrayRemove(a, iblock) ) { 
        /* In case of any unsuccessful freeing, you may
        .. expect memory related "unexpected behaviour"*/
        status = HMESH_ERROR;
        HmeshError("HmeshArrayDestroy() : "
                   "cannot free memblock");
      }
    }
  
  /* NOTE: since pool is a general pool for all attributes
  .. of same object_size, it will not be freed here. 
  .. MempoolFree(a->pool); 
  */
  if(a->address)     
    free(a->address);
  if( IndexStackDestroy(&a->stack) != HMESH_NO_ERROR ){
    HmeshError("HmeshArrayDestroy() : "
               "index stack cannot be cleaned. "
               "(Memory Leak) ");
  }
  free(a);

  return status;
}

/*
.. Add 1 block to all the attributes of cells
*/
static
_Flag HmeshCellsExpand(_HmeshCells * cells) {
  _Index iblock = IndexStackFreeHead(cells->blocks, 0);
  _Index nattr = cells->scalars.n;
  while(nattr--) {
    _Index iattr = cells->scalars.info[nattr].in_use;
    _HmeshArray * attr = (_HmeshArray *) cells->attr[iattr];
    if ( !attr ) { 
      HmeshError("HmeshCellsExpand() : "
        "attr[%d] not found", iattr);
      return HMESH_ERROR;
    }
    void * mem = HmeshArrayAdd(attr, iblock); 
    if ( !mem ) {
      HmeshError("HmeshCellsExpand() : cannot add block to "
        "attr '%s'", attr->name);
      return HMESH_ERROR;
    }
    if(!iattr) {
      _Index * prev = (_Index *) mem;
      prev[0] = 0;
    }
    else if(iattr == 1) {
      /* Free index list. Index '0' is reserved node*/
      _Index * next = (_Index *) mem, bsize = MemblockSize();
      for(_Index index = 1; index < bsize-1; ++index)
        next[index] = index + 1;
      next[bsize-1] = 0;
      /* Head of used list*/
      next[0] = 0;
    }
  }
  
  if(cells->max < cells->blocks->max) {
    cells->max = cells->blocks->max;
    cells->info = (_Index *) 
      realloc (cells->info, 4 * cells->max * sizeof(_Index));
  }

  _Index * info = cells->info + (4*iblock),
    bsize = (_Index) MemblockSize();

  /* head of in-use and free-list linked list respectively*/
  info[0] = 0;
  info[1] = 1;
  /* number of nodes in-use and free */ 
  info[2] = 0;
  info[3] = bsize - 1; 

  return HMESH_NO_ERROR;
}

/* 
.. Create a HmeshCells (list of vertices, edges, etc ..)
*/
_HmeshCells * HmeshCells(_Flag d, _Flag D) {
  _HmeshCells * cells = 
    (_HmeshCells *) malloc (sizeof(_HmeshCells));

  if( (!cells) || (d>3) || (d>>D) ) 
    return NULL;
  
  cells->attr = NULL;
  _IndexStack * scalars = &cells->scalars;
  *scalars = IndexStack(HMESH_MAX_NVARS,5, &cells->attr);

  /* 'prev' and 'next' is used to keep double linked 
  .. list of nodes */
  IndexStackAllocate(scalars, 0);
  _HmeshArray * prev = HmeshArray("prev", sizeof(_Index));
  cells->attr[0] = prev;
  IndexStackAllocate(scalars, 1);
  cells->attr[1] = HmeshArray("next", sizeof(_Index));
  
  cells->blocks = prev ? &prev->stack : NULL;
  cells->d = d;
  cells->max = 0;
  cells->info = NULL;

  if(d) {
    char v[] = "v0";
    /* sub cells. triangles/edges/vertices */
    v[0] = (d == 3) ? 't' : (d == 2) ? 'e' : 'v';  
    for(int _d=0; _d<=d; ++d) {
      IndexStackAllocate(scalars, _d + 2);
      cells->attr[_d + 2] = 
        HmeshArray(v, sizeof(_Node));
      v[1]++;
    }
    cells->min = 2 + (d+1);
  }
  else {
    char x[] = "x.x";
    /* Vector 'x' */
    for(int _d=0; _d<D; ++_d) {
      IndexStackAllocate(scalars, _d + 2);
      cells->attr[_d + 2] = 
        HmeshArray(x, sizeof(_Real));
      x[2]++;
    }
    cells->min = 2 + D;
  }

  for(_Index iattr = 0; iattr<cells->min; ++iattr) {
    if(cells->attr[iattr] == NULL)
      HmeshError("HmeshCells() : attribute missing");
  }

  /* Expand all attributes of cells by 1 block */
  HmeshCellsExpand(cells);

  return cells;
}

/* Destroy cells with all it's attributes */
_Flag HmeshCellsDestroy(_HmeshCells * cells) {
  _IndexStack * stack = &cells->scalars;
  void ** attr = cells->attr;
  _Index n = stack->n;
  _Flag status = HMESH_NO_ERROR;
  while(n--) {
    _Index iscalar = stack->info[n].in_use;
    _HmeshArray * s = (_HmeshArray *) attr[iscalar];
    status |= HmeshArrayDestroy(s);
  }
  free(cells->attr);
  free(stack->info);
  free(cells->info);
  free(cells);
  
  if(status) 
    HmeshError("HmeshCellsDestroy() : "
      "Warning! ugnoring error in destroying scalars");
  return status;
}

/* 
.. Add a scalar with name 'name' to the list of
.. attributes of 'cells'. NOTE: name length should be < 32,
.. and should follow C naming rules
*/
_HmeshArray * 
HmeshScalarNew(_HmeshCells * cells, char * name) {
  if(!name) {
    HmeshError("HmeshScalarNew() : no name specified");
    return NULL;
  }

  if(strlen(name) > 31) {
    HmeshError("HmeshScalarNew() : very long name");
    return NULL;
  }

  if (! (isalpha(name[0]) || name[0] == '_') ) { 
    HmeshError("HmeshScalarNew() : wrong naming");
    return NULL;
  }
  
  for(int i=1; name[i] != '\0'; ++i) {
    if( !(isalnum(name[0]) || name[0] == '_') ) {
      HmeshError("HmeshScalarNew() : wrong naming");
      return NULL;
    }
  }
  
  _IndexStack * stack = &cells->scalars;
  void ** attr = cells->attr;
  for(int i=cells->min; i< stack->n; ++i){
    _Index iscalar = stack->info[i].in_use;
    _HmeshArray * s = (_HmeshArray *) attr[iscalar];
    if(!strcmp(s->name, name)) {
      HmeshError("HmeshScalarNew() : scalar '%s' exists", name);
      return NULL;
    }
  }

  _Index iscalar = IndexStackFreeHead(stack, 1);
  if(iscalar == UINT16_MAX){
    HmeshError("HmeshScalarNew() : out of scalar index");
    return NULL;
  }

  _HmeshArray * s = HmeshArray(name, sizeof(_Real));
  if(!s) {
    HmeshError("HmeshScalarNew() : HmeshArray() failed");
    return NULL;
  }
  _IndexStack * blocks = cells->blocks;
  for(int i=0; i < blocks->n; ++i) {
    _Index iblock = blocks->info[i].in_use;
    if( !HmeshArrayAdd(s, iblock) ) {
      HmeshError("HmeshScalarNew() : "
        "HmeshArrayAdd() : failed");  
      HmeshArrayDestroy(s);
      return NULL;
    }
  }

  cells->attr[iscalar] = s;
    
  return s;
}

/* 
.. Remove the scalar 'name' from the attributes of 'cells'
*/
_Flag HmeshScalarRemove(_HmeshCells * cells, char * name) {
  if( !(name||cells) ) {
    HmeshError("HmeshScalarRemove() : aborted");
    return HMESH_ERROR;
  }
  
  _IndexStack * stack = &cells->scalars;
  void ** attr = cells->attr;
  _Index n = stack->n;
  while(n-- > cells->min) {
    _Index iscalar = stack->info[n].in_use;
    _HmeshArray * s = (_HmeshArray *) attr[iscalar];
    if(!strcmp(s->name, name)) {
      if(HmeshArrayDestroy(s)) {
        HmeshError("HmeshScalarRemove() : "
          "cannot remove scalar '%s'", name);
        return HMESH_ERROR;
      }
      attr[iscalar] = NULL;
      IndexStackDeallocate(stack, iscalar);
      return HMESH_NO_ERROR;
    }
  }

  HmeshError("HmeshScalarRemove() : scalar '%s' not found", name);
  return HMESH_ERROR;
}

/*
.. Add a node to the 'cells'
*/
_Node HmeshNodeNew(_HmeshCells * cells) {
  _IndexStack * blocks = cells->blocks;
  _Index iblock = blocks->info[blocks->n-1].in_use, 
    * prev = (_Index *)((_HmeshArray *) cells->attr[0])->address[iblock],
    * next = (_Index *)((_HmeshArray *) cells->attr[1])->address[iblock],
    head = cells->info[4*iblock], fhead = cells->info[4*iblock + 1];
  
  /* Make sure free head is not empty */
  assert(fhead);

  /* If this is the last free node in the block, add block */
  if(!next[fhead]) 
    /* fixme : Optimize a bit to reuse freed nodes in previous blocks */
    HmeshCellsExpand(cells);

  /* Remove fhead from free list, update free head*/
  cells->info[4*iblock+1] = next[fhead]; 

  /* adding fhead to used list */
  next[fhead] = next[head];
  prev[fhead] = head;
  next[head] = prev[next[head]] = fhead;
  cells->info[4*iblock] = next[fhead];

  /* Update count*/
  cells->info[4*iblock+2]++;
  cells->info[4*iblock+3]--;

  return (_Node) {.index = fhead, .iblock = iblock}; 
}

/*
.. Add a node to the 'cells'
*/
_Flag HmeshNodeRemove(_HmeshCells * cells, _Node node) {
  _Index iblock = node.iblock, index = node.index, 
    * prev = (_Index *)((_HmeshArray *) cells->attr[0])->address[iblock],
    * next = (_Index *)((_HmeshArray *) cells->attr[1])->address[iblock];

  /* Error : Index is already a free index, or the reserved index '0'
  .. or out of bound */
  if( (prev[index] == UINT16_MAX) || (!index) || 
      (index >= MemblockSize()) )
    return HMESH_ERROR;
  
  /* remove 'index' from used list */
  next[prev[index]] = next[index];
  prev[next[index]] = prev[index];
  cells->info[4*iblock] = prev[index];

  /* add 'index' to free list, update free head*/
  next[index] = cells->info[4*iblock+1];
  prev[index] = UINT16_MAX;
  cells->info[4*iblock+1] = index;

  /* Update count*/
  cells->info[4*iblock+2]--;
  cells->info[4*iblock+3]++;

  return HMESH_NO_ERROR;
}
