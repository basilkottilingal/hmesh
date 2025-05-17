#include <stdlib.h>
#include <stdio.h>

#include <memory.h>
  
/*
.. @ _AstPoolHandler : Datatype that stores all root information 
..    related to memory handling
*/
typedef struct {
  void ** blocks;
  int nblocks; 
  char * head;
  _AstPool ** pools;
  int npools; 
} _AstPoolHandler;

_AstPoolHandler _ast_pool_ = {0};
 
/*
.. Create a new memory block
*/ 
static void ast_memory_block ( void ) {
  _AstPoolHandler * p = &_ast_pool_;

  size_t size = _H_AST_BLOCK_SIZE_;

  while (size >= 4096) {

    void * mem = malloc(size);
    if (!mem) {
      size = size >> 1;
      continue;
    }
    
    p->nblocks++; 
    p->blocks = realloc(p->blocks, (p->nblocks)* sizeof(void *));
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
  _AstPoolHandler * p = &_ast_pool_;
  void ** blocks = p->blocks; 
  if(!blocks)
    return;

  int iblock = p->nblocks;
  while (iblock--) 
    free(blocks[iblock]);
  free(blocks);

  _AstPool ** pools = p->pools;
  int ipool = p->npools;
  while(ipool--)
    free(pools[ipool]);
  free(pools);

  p->blocks =  NULL;
  p->nblocks = 0; 
  p->head = NULL;
  p->pools =  NULL;
  p->npools = 0; 
}

/*
.. @ ast_allocate_internal() : API function (preferred to be used
.. internally) to allocate memory of size 'size' in [1, 4096]
*/
void * ast_allocate_internal ( size_t size ) {

  if(size > 4096) {
    fprintf(stderr, "ast_allocate_internal() : size too large");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  /*
  .. 8 Byte alignement.
  */
  size = (size + 7) & ~((size_t)7);

  _AstPoolHandler * p = &_ast_pool_;

  size_t available = !p->nblocks ? 0 :
    p->head - (char *) p->blocks[p->nblocks - 1];

  if( available < size ) 
    ast_memory_block();

  p->head -= size;

  return (void *) p->head;

}

typedef struct _FreeNode {
  struct _FreeNode * next;
} _FreeNode;

/*
..
*/

_AstPool * ast_pool (size_t size) {
  if( (size < sizeof(void *)) || (4096 % size) ){
    /*
    .. This function is used for creating memory pool for 
    .. derived datatypes, carefully made with proper alignment.
    .. For basic datatypes, you can pool a whole array using
    ..   char * arr = (char *) ast_allocate_internal(4096);
    */
    fprintf(stderr, "ast_pool() : bad alignment.");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  _AstPool * pool = (_AstPool *) malloc (sizeof(_AstPool *));

  _AstPoolHandler * p = &_ast_pool_;
  p->npools++; 
  p->pools =  realloc (p->pools, (p->npools)* sizeof(_AstPool*));
  p->pools[p->npools - 1] = pool;

  /*
  .. allocate a page and divide it into 4096/size FreeNode(s)
  */
  pool->size = size;
  pool->fhead = NULL;

  char * page = (char *) ast_allocate_internal(4096);
/*
  _FreeNode * fhead = NULL,
    * node = (_FreeNode *) page;
  for(int i = -(4096/size); i; ++i) {
    node->next  = fhead;
    page += size;
  }
  pool->fhead = node;
*/
  return pool;
}

