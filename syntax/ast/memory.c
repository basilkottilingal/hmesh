#include <stdlib.h>
#include <stdio.h>

#include <memory.h>
  
/*
.. @ _AstPool : Datatype that stores all root information 
..    related to memory handling
*/
typedef struct {
  void ** blocks;
  char * head;
  int nblocks; 
} _AstPool;
_AstPool _ast_pool_ = {0};
 
/*
.. Create a new memory block
*/ 
static void ast_memory_block ( void ) {
  _AstPool * p = &_ast_pool_;

  size_t size = _H_AST_BLOCK_SIZE_;

  while (size >= 4096) {

    void * mem = malloc(size);
    if (!mem) {
      size = size >> 1;
      continue;
    }
    
    p->nblocks++; 
    p->blocks = (void **) realloc(p->blocks, (p->nblocks)* sizeof(void *));
    p->blocks[p->nblocks - 1] = mem;
    p->head = (char *) mem + size;

    return;
    
  } 

  fprintf(stderr, "ast_memory_block() : failed!");
  fflush(stderr);
  exit(EXIT_FAILURE);

  return;
  
}

/*
.. Deallocates all blocks. Should be called at the end of the pgm
*/
void ast_deallocate_all() {
  _AstPool * p = &_ast_pool_;
  void ** blocks = p->blocks; 
  if(!blocks)
    return;

  int iblock = p->nblocks;
  while (iblock--) 
    free(blocks[iblock]);
  free(blocks);

  p->blocks =  NULL;
  p->head = NULL;
  p->nblocks = 0; 
}

/*
.. @ ast_allocate() : API to allocate memory of size 'size' .
*/
void * ast_allocate ( size_t size ) {

  if(size > 4096) {
    fprintf(stderr, "ast_allocate() : size too large");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  _AstPool * p = &_ast_pool_;

  size_t available = !p->nblocks ? 0 :
    p->head - (char *) p->blocks[p->nblocks - 1];

  if( available < size ) 
    ast_memory_block();

  p->head -= size;

  return (void *) p->head;

}
