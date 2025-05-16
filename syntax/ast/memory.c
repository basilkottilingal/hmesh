#include <stdlib.h>
#include <stdio.h>

#include <memory.h>
  
/*
.. @ _AstPool : Datatype that stores all root information 
..    related to memory handling
*/
typedef struct {
  void ** blocks, * head;
  int nblocks; 
} _AstPool;
_AstPool _H_AST_POOL_ = {0};
 
/*
.. Create a new memory block
*/ 
static void ast_memory_block ( void ) {
  _AstPool * m = &_H_AST_POOL_;

  size_t size = _H_AST_BLOCK_SIZE_;

  while (size >= 4096) {

    void * mem = malloc(size);
    if (!mem) {
      size = size >> 1;
      continue;
    }
    
    p->nblocks++; 
    p->blocks = (void **) realloc(p->blocks, (p->nblocks)* sizeof(void *));
    p->blocks[max-1] = mem;
    p->head = mem + size;

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
  _AstPool * p = &_H_AST_POOL_;
  void ** blocks = p->blocks; 
  if(!blocks)
    return;

  int iblock = 1 + p->nblocks;
  while (iblock--) 
    free(blocks[iblock])
  free(blocks);

  p->blocks  = p->head = NULL;
  p->nblocks = 0; 
}

/*
.. @ ast_allocate() : API to allocate memory of size 'size' .
*/
void ast_allocate ( size_t size ) {

  if(size > 4096) {
    fprintf(stderr, "ast_allocate() : size too large");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  _AstPool * p = &_H_AST_POOL_;

  if( !p->nblocks ) 
    ast_memory_block();

  if(p->head - p->blocks[p->nblocks - 1] < size) 
    ast_memory_block();

  p->head -= size;

  return p->head;

}
