/* Add a block @ iblock-th position.
.. Return memory address of the block, if successful
*/
void * 
HmeshAttributeAdd(_HmeshAttribute * a, _Flag iblock) {

  /* handling unexpected behaviour */
  if (iblock >= HMESH_MAX_NBLOCKS) {
    HmeshError("HmeshAttributeAdd() : iblock out of bound");
    return NULL;
  }
  if(iblock < a->n) {
    /* In case a block already exist @ this location */
    if(address[iblock]) {
      HmeshError("HmeshAttributeAdd() : block already in use");
      return address[iblock];
    }
  }
  else {
    a->n = iblock + 1;
    a->address = 
      (void **) realloc(a->address, (a->n)*sizeof(void *));
    for(int i=a->n; i<iblock; ++i)
      address[i] = NULL;
  }

  /* get a block from pool */
  _Memblock b = _MempoolAllocateFrom(a->pool);
  void * m = MemblockAddress(b);
  if(!m) {
    HmeshError("HmeshAttributeAdd() : memory pooling failed");
    return NULL;
  }

  /* This information is used for deallocation of this block */
  a->i[iblock] = b.iblock;
  a->address[iblock] = m;
  return m;
}

/* deallocate an existing block @ iblock-th position 
.. Return HMESH_NO_ERROR if successful*/
_Flag 
HmeshAttributeRemove(_HmeshAttribute * a, _Flag iblock){

  _Flag status = iblock < a->n ? (address[iblock] != NULL) : 0;
  if(!status) {
    HmeshError("HmeshAttributeRemove() : "
               "cannot locate memory block");
    return HMESH_ERROR;
  }

  status = MempoolDeallocateTo( (_Memblock) 
            {.pool = a->pool, .iblock = a->i[iblock]});

  if(status == HMESH_NO_ERROR)  
    address[iblock] = NULL;
  else 
    HmeshError("HmeshAttributeRemove() : "
               "MempoolDeallocateTo() : failed");

  return status;
}

/* Create a new attribute */
_HmeshAttribute * 
HmeshAttribute(char * name, size_t size, _Mempool * p) {

  _Mempool * pool = !p ? NULL :
    (p->object_size != size) ? NULL : p;
  if(!p) {
    HmeshError("HmeshAttribute() : aborted");
    return NULL;
  }

  /* A new attribute */
  _HmeshAttribute * a = 
    (_HmeshAttribute *)malloc(sizeof(_HmeshAttribute));
  a->pool = pool;
  a->address = NULL;
  a->n = 0;
  /* NOTE : For scalars 'name' is necessary */
  if(!name) { 
    HmeshError("HmeshAttribute() : "
               "Warning. attribute with no specified name");
    a->name = NULL;
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
HmeshAttributeDestroy(_HmeshAttrribute * a) {

  _Flag status = HMESH_NO_ERROR;
  /* Remove all blocks in use */
  for(int i=0; i<a->n; ++i) 
    if(address[i]) {
      if(HmeshAttributeRemove(a, i) != HMESH_NO_ERROR ) { 
        status = HMESH_ERROR;
        HmeshError("HmeshAttributeDestroy() : "
                   "cannot free block");
      }
      /* probable Memory leak, 
      .. in case of any unsuccessful freeing */
      address[i] = NULL;
    }
  
  /* NOTE: since pool is a general pool for all attributes
  .. of same object_size, it will not be freed here. 
  */
  if(address)
    free(address); 
  free(a);

  return status;
}

