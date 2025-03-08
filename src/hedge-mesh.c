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

_Flag
HmeshAttributeAccomodate(_HmeshAttribute * nodes,
  _HmeshAttribute * a) 
{
  if(!(nodes && a)) {
    HmeshError("HmeshAttributeAccomodate() : aborted");
    return HMESH_ERROR;
  }

  for(int i=0; i< nodes->n; ++i) {
    _Flag yes = (i < a->n) ? (a->address[i] != NULL) : 1; 
    if ( yes )
      if( !HmeshAttributeAdd(a, i) ) {
        HmeshError("HmeshAttributeAccomodate() : "
                   "HmeshAttributeAdd() : failed");
        return HMESH_ERROR;
      }
  }

  return HMESH_NO_ERROR;
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

/* add a new scalar 'name' to the cells 'c' 
*/
_Flag
_HmeshCellsAddScalar(_HmeshCells * c, char * name) {

  size_t len = name ? strlen(name) : 0;  
  if( (!len) || (len > HMESH_MAX_VARNAME) || 
      (c->nscalars == HMESH_MAX_NVARS) ){
    HmeshError("HmeshCellsAddScalar() : aborted()");
    return HMESH_ERROR;
  }

  for(int i=0; i < c->nscalars; ++i) {
    _HmeshScalar * s = c->s[i];
    if(!s) continue;
    if(! strcmp( name, s->name) ) {
      char err[100];
      sprintf(err, "HmeshCellsAddScalar() : scalar with name "
              "'%s' exists", name);
      HmeshError(err);
      return HMESH_ERROR;
    }
  }
  _HmeshScalar * s = (_HmeshScalar *) 
    HmeshAttribute(name, sizeof(_DataType), HMESH_SCALAR_POOL);
  if(!s) return HMESH_ERROR;
  
  c->nscalars += 1;
  c->s = (_HmeshScalar **) 
    realloc((c->nscalars)*sizeof(_HmeshScalar *));
  c->s[c->nscalars - 1] = s;

  HmeshAttributeAccomodate(c->nodes, s);

  return HMESH_NO_ERROR;
}

/* remove scalar 'name' from the cells 'c' 
*/
_Flag
_HmeshCellsAddScalar(_HmeshCells * c, char * name) {

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
      if( HmeshAttributeDestroy((_HmeshAttribute *)s) !=
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

  HmeshError("HmeshCellsRemoveScalar() : scalar not found");
  return HMESH_ERROR;
}

_HmeshNode * HmeshNodeNew(_HmeshNodeBlock * block) {

  /* 'NodeNew(block)' : return an available node
  .. in the 'block', (if any)
  */

  /* In case there is no empty nodes available,
  .. return NULL*/ 
  if (!block->empty) 
    return NULL;

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
