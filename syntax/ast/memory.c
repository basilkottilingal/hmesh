#include <memory.h>

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
  }

  size_t size = 1<<20;

  do {
    void * mem = malloc(size);
    if (!mem)
      continue;
    
    m->max++; 
    m->blocks = (_Block **) realloc(m->blocks, (m->max)* sizeof(_Block *));
    m->blocks[max-1] = mem; 

    return mem;
    
  } while (0);

  fprintf(stderr, "ast_memory_block() : malloc() failed!");
  fflush(stderr);
  return NULL;
  
}

#define _H_AST_PTR_SAFETY 0xFBC9183

/*
.. API to return a page.
.. return type will be page number [0, _H_AST_PAGE_SIZE).
.. This function will exit(-1), if couldn't allocate a page. 
*/
_H_AST_PTR_ ast_allocate_page() {
  _MemoryHandler * m = &_MemoryHandler_;

  _
  
  if(m->fhead) {
    _FreePage * next = m->fhead->next;
    m->fhead->safety = 0;
    _Page * page = (_Page*) m->fhead;
    m->fhead = next;
  }
    
  }
}
