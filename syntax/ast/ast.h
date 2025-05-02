/*
.. @Ast : represent an Abstract Syntaxt Tree (AST) node
*/
typedef struct _Ast {
  /*
  .. node connection in the AST tree. 
  */
  void * child, * sibling, * parent;
    
  /* info on the node:
  .. (a) is it leaf/internal node
  */
  unsigned char type;

  /* line number of the token (if it's a leaf node) */
  int lineno;

  /* token id, or grammar id */
  int symbol;
  
} _Ast;

/* 
.. NOTE: including path, source file and headernames
.. are limited to 1024 characters if not overridden
*/
# ifndef _HMESH_PARSER_FILENAME_MAX
# define _HMESH_PARSER_FILENAME_MAX_ 1024
# endif

/*
.. when you use "pure" parser i,e if you have set api.pure as full, 
.. you don't use any global variables. Alternative is 
.. to use arguements that you pass to yyparse() &/ yylex().
.. An AstGlobal obj can store global states avoiding .. ?
*/
typedef struct _AstGlobal {
  /* 
  .. stores file name info. 
  */
  char source[_HMESH_PARSER_FILENAME_MAX_];

  /* The root node : usually, it is the  */ 
  _Ast * root;

  /* Stack: for identifiers 
  _Stack 
  */

} _AstGlobal;

/* Identify attribute of a ast node */
enum {
  
  /* Is it a leaf node */
  AstNodeIsLeaf =  1,

  /* An internal node */
  AstNodeIsInternal = 2
};

/*
.. Get a new Ast Node. 
.. fixme: Use memory pool.
*/

extern _Ast * AstNode(_Ast *,int , int , unsigned char);
