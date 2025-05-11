#include <memory.h>
 
#define _H_AST_PTR_B_(_i_)  ( _i_ >> 20 )
#define _H_AST_PTR_P_(_i_)  ( (_i_ >> 12) & 511 )
#define _H_AST_PTR_N_(_i_)  ( _i_ & 8191 )

#define _H_AST_PTR_ENCODE_(_b_,_p_,_n_) \
  (  (_b_<<20) | (_p_<<12) | (_n_)  )

#define _H_AST_PTR_ADDRESS_(_i_)                                  \
  (  ((char *) _MemoryHandler_.blocks[ _H_AST_PTR_B_(_i_) ] )  +  \
     (_i_ & _H_AST_20_BITS_)  )

/*
.. Global variable, for all memory handling.
.. Keeps root information about memory blocks,
.. number of blocks createad and a free list of pages.
.. NOTE, fixme : NOT Thread safe!.
*/
_MemoryHandler _MemoryHandler_ = {0};

/*
.. Create a new memory block
.. fixme : try malloc() with smaller size, if 1 MB fails.
*/ 
static void * ast_memory_block ( void ) {
  _MemoryHandler * m = &_MemoryHandler_;
  if(m->fhead) {
    fprintf(stderr, "ast_memory_block() : "
      "Warning : free pages already available!");
    return NULL;
  }

  size_t size = _H_AST_BLOCK_SIZE_;

  do {
    void * mem = malloc(size);
    if (!mem) {
      size = size >> 1;
      continue;
    }
    
    m->max++; 
    m->blocks = (void **) realloc(m->blocks, (m->max)* sizeof(void *));
    m->blocks[max-1] = mem; 

    return mem;
    
  } while (0);  /* replace with size > _H_AST_PAGE_SIZE*/

  fprintf(stderr, "ast_memory_block() : malloc() failed!");
  fflush(stderr);
  exit(EXIT_FAILURE);

  return NULL;
  
}

/*
.. Deallocates all blocks. Should be called at the end of the pgm
*/
void ast_deallocate_all() {
  _MemoryHandler * m = &_MemoryHandler_;
  void ** blocks = m->blocks; 
  _Mempool ** pools = m->pools;
  if(!blocks)
    return;

  for(int i=0; i<m->npools; ++i)
    free(pools[i]);
  for(int i=0; i<m->max; ++i)
    free(blocks[i]);
  free(pools);
  free(blocks);

  *m = {0};
}

#define _H_AST_PTR_SAFETY 0xFBC9183

/*
.. @ ast_allocate_page() : API to return a page.
..    This function will exit(-1), if couldn't allocate a page. 
..    return type : _H_AST_PTR_ (alias uint32_t) 
..    returns     : page number [0, _H_AST_PAGE_SIZE) .
*/
_H_AST_PTR_ ast_allocate_page() {
  _MemoryHandler * m = &_MemoryHandler_;

  if(!m->fhead) {
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
