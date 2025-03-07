void * HmeshAttributeAdd(_HmeshAttribute * a, _Flag iblock) {
  if(iblock < a->n) {
    if(address[iblock]) {
      HmeshError("HmeshAttributeAdd() : iblock in use");
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
  _Memblock b = _MempoolAllocateFrom(a->pool);
  void * m = MemblockAddress(b);
  if(!m) {
    HmeshError("HmeshAttributeAdd() : pool out of memory");
    return NULL;
  }
  address[iblock] = m;
  return m;
}

_Flag HmeshAttributeRemove(_HmeshAttribute * a, _Flag iblock){
  _Flag status = iblock < a->n ? (address[iblock] != NULL) : 0;
  if(!status) {
    HmeshError("HmeshAttributeRemove() : "
               "cannot locate memory block");
    return HMESH_ERROR;
  }
  if(
}
