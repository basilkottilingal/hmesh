#ifndef _H_AST_SCOPE_
#define _H_AST_SCOPE_

  #include <hash.h>

  typedef struct _Scope _Scope;

  /*
  .. API funcs
  .. (a) create a new scope inside 'parent' scope
  .. (b) pop() from the scope 'scope'
  */
  extern _Scope * scope_new ( _Scope * parent );
  extern _Scope * scope_pop ( _Scope * scope );

#endif
