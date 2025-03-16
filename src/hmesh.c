#include <common.h>
#include <mempool.h>
#include <hmesh.h>

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
  _Memblock b = MempoolAllocateFrom(a->pool);
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

/* Make sure that attribute 'a' has same number of
.. blocks as that of the 'reference' array.
*/
static inline _Flag
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
  a->stack = IndexStack(HMESH_MAX_NBLOCKS, 4, &a->address);
  a->max  = a->stack.max;
  if(a->max) 
    a->iblock = (_Index *) malloc (a->max * sizeof(_Index));

  /* NOTE : For scalars 'name' is necessary */
  if(!name ? 1 : !name[0]) { 
    HmeshError("HmeshArray() : Warning! No name specified");
    a->name[0] = '\0';
    return a;
  }

  /* Set attribute name. 
  .. Length of name is restricted to 32 including '\0' 
  .. fixme : make sure naming follows C naming rules */
  _Flag s = 0;
  char * c = name;
  while(*c && s<31)
    a->name[s++] = *c++;
  a->name[s] = '\0';

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
        status = HMESH_ERROR;
        HmeshError("HmeshArrayDestroy() : "
                   "cannot free memblock");
      }
      if( IndexStackDeallocate(&a->stack, a->iblock[iblock]) ){
        status = HMESH_ERROR;
        HmeshError("HmeshArrayDestroy() : "
                   "cannot free block index");
      }

      /* In case of any unsuccessful freeing, you may
      .. expect memory related "unexpected behaviour"*/
      a->address[iblock] = NULL;
    }
  
  /* NOTE: since pool is a general pool for all attributes
  .. of same object_size, it will not be freed here. 
  */
  if(a->address)     
    free(a->address);
  if( IndexStackDestroy(&a->stack) != HMESH_NO_ERROR ){
    HmeshError("HmeshArrayDestroy() : "
               "index stack cannot be cleaned. "
               "(Memory Leak) ");
  }
  MempoolFree(a->pool);
  free(a);

  return status;
}

static
_Flag HmeshCellsExpand(_HmeshCells * cells) {
  _Index iblock = IndexStackFreeHead(cells->blocks, 1);
  _Index nattr = cells->scalars.n;
  while(nattr--) {
    _Index iattr = cells->scalars.info[nattr].in_use;
    _HmeshArray * attr = (_HmeshArray *) cells->attr[iattr];
    if ( !attr ) { 
      HmeshError("HmeshCellsExpand() : attr[%d] not found", iattr);
      return HMESH_ERROR;
    }
    if ( HmeshArrayAdd(attr, iblock) ) {
      HmeshError("HmeshCellsExpand() : cannot add block to "
                 "attr '%s'", attr->name);
      return HMESH_ERROR;
    }
  }
  
  _Index * prev = 
    (_Index *) (((_HmeshArray *) cells->attr[0])->address[iblock]),
    * next = 
    (_Index *) (((_HmeshArray *) cells->attr[1])->address[iblock]);

  return HMESH_NO_ERROR;
}

_HmeshCells * HmeshCells(_Flag d) {
  _HmeshCells * cells = 
    (_HmeshCells *) malloc (sizeof(_HmeshCells));

  if(!cells) return NULL;
  
  cells->attr  = NULL;
  _IndexStack * scalars = &cells->scalars;
  *scalars = IndexStack(HMESH_MAX_NVARS,5, &cells->attr);

  _HmeshArray * prev = HmeshArray("prev", sizeof(_Index));
  if(!prev) {
    HmeshError("HmeshCells() : creating 'prev' failed");
    free(cells);
    return NULL;
  }
  _HmeshArray * next = HmeshArray("next", sizeof(_Index));
  if(!next) {
    HmeshError("HmeshCells() : creating 'next' failed");
    HmeshArrayDestroy(prev);
    free(cells);
    return NULL;
  }

  /* 'prev' and 'next' is used to keep double linked list of nodes */
  IndexStackAllocate(scalars, 0);
  cells->attr[0] = prev;
  IndexStackAllocate(scalars, 1);
  cells->attr[1] = next;
  
  cells->blocks = &prev->stack;
  cells->d = d;

  return cells;
}
