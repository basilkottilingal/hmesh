#include <scope.h>
#include <hash.h>
#include <memory.h>

  
struct _DeclList {
  struct DeclList * next, * prev;
  _HashNode * h;
};

struct _Scope {
  int scope;
  struct DeclList

  _HashTable * symbols;
  struct _Scope * next;
};


static
_Scope * freelist = NULL;

_Scope * scope_new ( _Scope * parent ) {
  int scope = parent ? (parent->scope + 1) : 0;

  if ( freelist ) {
    _Scope * scope = freelist;
    freelist = scope->next;
    assert ( scope->symbols && scope->scope == scope );
    scope->next = parent;
    return scope;
  }

  _Scope * scope = ast_allocate_general (sizeof (_Scope));
  scope->scope = scope; 
  scope->next = parent;
  /*
  .. symbol table size is 
  .. 128 for scope 0, 
  .. 64  for scope 1, and 
  .. 32  for all higher scopes > 1
  */
  scope->symbols = 
    hash_table_init ( scope ? (scope == 1 ? 6 : 5 ) : 8 );

  return scope;
}

_Scope * scope_pop ( _Scope * current) {
  _Scope * parent = current->parent;
  current->next = freelist;
  freelist = current;
}


