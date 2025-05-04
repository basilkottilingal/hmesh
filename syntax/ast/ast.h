# ifndef _HMESH_AST_
# define _HMESH_AST_

/*
.. -------------------------------------
.. --------- AST Node ------------------
.. -------------------------------------
*/

/*
.. @AstNode : represent a node of the Abstract Syntaxt Tree (AST)
*/
typedef struct _AstNode {
  /*
  .. node connection in the AST tree. 
  */
  struct _AstNode * child, * sibling, * parent;
    
  /* info on the node:
  .. (a) is it leaf/internal node
  */
  unsigned char type;

  /* line number of the token (if it's a leaf node) */
  int lineno;

  /* token id, or grammar id */
  int symbol;
  
} _AstNode;

/* 
..  Identify attribute of an ast node 
*/
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

extern _AstNode * AstNode(_AstNode *,int , int , unsigned char);
/*
.. -------------------------------------
.. --------- AST -----------------------
.. -------------------------------------
*/


/* 
.. NOTE: length of the name of source file /include header file
.. are limited to 1024 characters (including the path and trailing '\0') 
.. if not overridden like -D_HMESH_PARSER_FILENAME_=4096
*/
# ifndef _HMESH_PARSER_FILENAME_MAX
# define _HMESH_PARSER_FILENAME_MAX_ 1024
# endif

/*
.. when you use "pure" parser i,e if you have set api.pure as full, 
.. you don't use any global variables. As an Alternative,
.. we use arguements that you pass to yyparse() &/ yylex().
.. An _Ast obj store the root node, and also other global states associated with the tree.
.. It is passed as an arguement in the reentrant functions like yyparse() and yylex().
*/
typedef struct _Ast {
  /* 
  .. stores file name info. 
  */
  char source[_HMESH_PARSER_FILENAME_MAX_];

  /* The root node : usually, it is the  */ 
  _AstNode * root;

  /* Stack: for identifiers 
  _Stack 
  */

} _Ast;


extern _Ast * AstInit(const char *);
extern void AstResetSource(_Ast * , const char * );

#endif   /* End of _HMESH_AST */
