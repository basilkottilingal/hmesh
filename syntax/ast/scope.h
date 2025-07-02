#ifndef _H_AST_SCOPE_
#define _H_AST_SCOPE_

  #include <hash.h>

  typedef struct _Scope {
    int depth;
    int cleared;
    int id;
    _HashTable * symbols;
    struct _Scope * parent, * child;
  } _Scope;

  /*
  .. API funcs
  .. (a) create a new scope inside 'parent' scope
  .. (b) pop() from the scope 'scope'
  */
  extern _Scope * scope_push ( _Scope * parent );
  extern _Scope * scope_pop ( _Scope * scope, int );
  extern void     scope_clear ( _Scope * scope );

#endif
