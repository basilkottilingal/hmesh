#include <stdlib.h>
#include <stdio.h>

#include <memory.h>
 

/*
.. Create a new memory block
.. fixme : try malloc() with smaller size, if 1 MB fails.
*/ 
static void * ast_memory_block ( void ) {
  _MemoryHandler * m = &_H_AST_POOL_;

  size_t size = _H_AST_BLOCK_SIZE_;

  do {
    void * mem = malloc(size);
    if (!mem) {
      size = size >> 1;
      continue;
    }
    
    m->nblocks++; 
    m->blocks = (void **) realloc(m->blocks, (m->nblocks)* sizeof(void *));
    m->blocks[max-1] = mem; 

    return mem;
    
  } while (0);  /* replace with size > _H_AST_MINIMUM_BLOCK_SIZE*/

  fprintf(stderr, "ast_memory_block() : malloc() failed!");
  fflush(stderr);
  exit(EXIT_FAILURE);

  return NULL;
  
}

/*
.. Deallocates all blocks. Should be called at the end of the pgm
*/
void ast_deallocate_all() {
  _MemoryHandler * m = &_H_AST_POOL_;
  void ** blocks = m->blocks; 
  if(!blocks)
    return;

  _H_AST_PTR_ iblock = 1 + _H_AST_PTR_B_(m->head);
  while (iblock--) 
    free(blocks[iblock])
  free(blocks);

  m->blocks = NULL;
  m->head   = 0; 
}

/*
.. @ ast_allocate() : API to allocate memory of size 'size' .
..    return type : _H_AST_PTR_ (alias uint32_t) 
*/
_H_AST_PTR_ ast_allocate ( size_t size ) {

  if(size > 4096) {
    fprintf(stderr, "ast_allocate() : size too large");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  _MemoryHandler * m = &_H_AST_POOL_;
  
  _H_AST_PTR_ iblock = _H_AST_PTR_B_ (m->head),
    head = _H_AST_PTR_N_ (m->head);

  if() {
    /* 
    .. In case we run out of free pages, create a new block
    .. and divide it into pages
    */
    char * page = ast_memory_block();
    _H_AST_PTR_ iblock = m->max - 1;
    _H_AST_PTR_ npages = _H_AST_BLOCK_SIZE_ / _H_AST_PAGE_SIZE_;
    for(_H_AST_PTR_ ip=0; ip<npages; ++ip, page += _H_AST_PAGE_SIZE_) {
      _FreePage * fpage = (_FreePage *) page;
      fpage->ipage  = _H_AST_PTR_ENCODE_ (iblock, ip, 0);
      fpage->safety = _H_AST_PTR_SAFETY;
      fpage->next   = m->fhead;
      m->fhead      = fpage;
    }
  }
 
  /* 
  .. Pop a page from free head
  */ 
  _FreePage * fpage = m->fhead;
  m->fhead = fpage->next;
  m->fhead->safety = 0;
  _Page * page = (_Page*) m->fhead;
    m->fhead = next;
}
