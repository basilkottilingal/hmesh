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
