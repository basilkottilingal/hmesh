#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <memory.h>

#define _AST_PAGE_SIZE_ ( (size_t) 4096 )

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
  char * str;
  size_t nchar;
} _AstPoolHandler;

_AstPoolHandler _ast_pool_ = {0};
 
/*
.. Create a new memory block.
*/ 
static void ast_memory_block ( void ) {
  _AstPoolHandler * p = &_ast_pool_;

  size_t size = _H_AST_BLOCK_SIZE_;

  while (size >= _AST_PAGE_SIZE_) {

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
.. @ ast_allocate_general() : API function (preferred to be used
.. internally) to allocate memory of size 'size' in [1, 4096]
.. NOTE : 'size' will be rounded off to 8 Byte alignement.
*/
void * ast_allocate_general ( size_t size ) {

  if(size > _AST_PAGE_SIZE_) {
    fprintf(stderr, "ast_allocate_general() : size too large");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  size = (size + 7) & ~((size_t) 7);

  _AstPoolHandler * p = &_ast_pool_;
  size_t available = !p->nblocks ? 0 :
    p->head - (char *) p->blocks[p->nblocks - 1];
  if( available < size ) 
    ast_memory_block();

  p->head -= size;
  return p->head;

}

typedef struct _FreeNode {
  struct _FreeNode * next;
} _FreeNode;

static void ast_pool_allocate_page (_AstPool * pool) {

  char * page = 
    (char *) ast_allocate_general(_AST_PAGE_SIZE_);
  _FreeNode * fhead = (_FreeNode * ) pool->fhead;

  size_t size = pool->size, n = _AST_PAGE_SIZE_/size;
  for(size_t i = 0; i < n; ++i) {
    _FreeNode * node = (_FreeNode *) page;
    page += size;
    node->next  = fhead;
    fhead = node;
  }
  pool->fhead = fhead;
}

/*
.. Create a pool, and maintain nodes a free list of
.. small memory chunks each with size 'size'.
..
.. NOTE : 8 BYTES alignement is very recommnded. 
.. This function is used for creating memory pool for faster
.. allocation  derived datatypes, carefully made with proper.
.. alignment. For array of basic datatypes, you may pool the 
.. whole array as
..   char * arr = (char *) ast_allocate_general(1024);
*/

_AstPool * ast_pool (size_t size) {
  if( size < sizeof(void *) || size > _AST_PAGE_SIZE_){
    fprintf(stderr, "ast_pool() : bad datasize");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  if(size & 7) {
    fprintf(stderr, "ast_pool() : WARNING !! bad alignment.");
    fflush(stderr);
  }

  _AstPool * pool = malloc (sizeof(_AstPool));

  _AstPoolHandler * p = &_ast_pool_;
  p->npools++; 
  p->pools = realloc (p->pools, (p->npools)* sizeof(_AstPool*));
  p->pools[p->npools - 1] = pool;

  pool->size = size;
  pool->fhead = NULL;

  ast_pool_allocate_page(pool);

  return pool;
}

/*
.. APIs to allocate a node from/deallocate a node to the pool
*/

void * ast_allocate_from (_AstPool * pool) {
  if(!pool->fhead)
    ast_pool_allocate_page(pool);
  _FreeNode * fhead = (_FreeNode *) pool->fhead;
  pool->fhead = fhead->next;
  return fhead;
}

void ast_deallocate_to (_AstPool * pool, void * fnode) {
  ((_FreeNode *) fnode)->next = (_FreeNode *) (pool->fhead);
  pool->fhead = fnode;
}

/*
.. strdup() equivalent to pool memory for string. No need/way to 
.. free() each strings, they all will be freed at the end when 
.. ast_deallocate_all() is called. A page is allocated to pool
.. each string, and new page is allocated once the previous one 
.. runs out.
.. NOTE : usually expected strlen is <= 31 (excluding '\0')
.. which is the identifier name size limit in most compilers.
.. However a 4096 including '\0'  set as the limit.
*/
char * ast_strdup ( const char * original ) {
  size_t len = strlen ( original ) + 1; 
  if(len > _AST_PAGE_SIZE_ )
    return NULL;

  _AstPoolHandler * p = &_ast_pool_;
  if (p->nchar < len ) {
    p->str =  (char * ) ast_allocate_general(_AST_PAGE_SIZE_);
    p->nchar = _AST_PAGE_SIZE_;
  }

  char * str = p->str;
  memcpy ( str, original, len );
  p->str   += len;
  p->nchar -= len;

  return str;
}
