#include <stdlib.h>
#include <assert.h>

#include <scope.h>
#include <hash.h>
#include <memory.h>

_Scope * scope_push ( _Scope * parent ) {

  if ( parent && parent->child )
    return parent->child;

  _Scope * scope = ast_allocate_general (sizeof (_Scope));
  int depth = parent ? (parent->depth + 1) : 0;
  scope->depth = depth; 
  scope->parent = parent;
  scope->child = NULL;
  if ( parent )
    parent->child = scope;
    
  /*
  .. default symbol table size is 
  .. 128 for scope 0, 
  .. 64  for scope 1, and 
  .. 32  for all higher scopes > 1
  */
  scope->symbols = 
    hash_table_init ( depth ? (depth == 1 ? 6 : 5 ) : 8 );

  return scope;
}

_Scope * scope_pop ( _Scope * scope, int clear) {
  /*
  .. fixme : not ideal. resetting hashtable is costly.
  .. Also, if hashtable has resized, it is recommended to 
  .. to shrink to the original size
  */
  if (clear)
    hash_table_reset ( scope->symbols );
  scope->cleared = clear;
  return scope->parent;
}

void scope_clear ( _Scope * scope ) {
  /*
  .. Reset child scope's symbol table
  */
  _Scope * child = scope->child;
  assert ( child );
  if ( !child->cleared ) {
    hash_table_reset ( child->symbols );
    child->cleared = 1;
  }
}
