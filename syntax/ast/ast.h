# ifndef _H_AST_
# define _H_AST_

  #include <memory.h>
  #include <hash.h>

  #ifndef _H_AST_STACK_SIZE_
    #define _H_AST_STACK_SIZE_ 128
  #endif
    
  /*
  .. Datatype to store, file name and line & column number of the source.
  */
    
  typedef struct _AstLoc {
    /* 
    .. NOTE: length of the name of source file /include header file
    .. are limited to _PAGE_SIZE_ = 4kB 
    */
    char * source;
    int line, column;
  } _AstLoc;

  /* 
  .. Macro that update ast->loc (i.e parser location) when a character
  .. is parsed. Assumes tab width = 8.
  .. Doesn't handle other non-common escape keys.
  */
  #define AstLocInput( __c__ )                         \
    do {                                               \
      if ( (__c__) == '\n' ) {                         \
        ++(ast->loc.line);                             \
        ast->loc.column = 1;                           \
      }                                                \
      else if ( (__c__) == '\t' ) {                    \
        ast->loc.column += 8 - (ast->loc.column) % 8;  \
      }                                                \
      else {                                           \
        ++(ast->loc.column);                           \
      }                                                \
    } while ( 0 ) ;

  /*
  .. Macro that updates parser loc after going through a full token.
  .. A small overhead, especially for longer token. O (n). 
  .. Use this ONLY for tokens that may contain escape keys.
  .. For every other characters you can update column by yyleng.
  */
  #define AstLocRead( __text__ )   \
    do {                           \
      char * c = __text__;         \
      while (*c) {                 \
        AstLocInput ( *c );        \
        ++c;                       \
      }                            \
    } while ( 0 );

  
  /*
  .. structs for 
  .. (a) an internal AST node which represents a grammaar rule.
  .. (b) a terminal AST node which represents a token like keyword
  .. (c) metadata of the AST tree 
  */
  typedef struct _AstNode {
    struct _AstNode * parent,
      ** child;
    int symbol;
  } _AstNode;

  typedef struct {
    _AstNode node;
    char *   token;
    _AstLoc  loc;
  } _AstTNode ;

  typedef struct _Ast {
    _AstNode      root;
    _AstNode *    iterator;
    _AstLoc       loc;
    _AstPool *    nodes;
    _AstPool *    tnodes;
    _HashTable *  identifiers;
  } _Ast;

  /*
  .. AST subtree traversal.
  .. Use Stack based traversal & Not recursive.
  .. WARNING : The root node passed (_root_) won't be traversed.
  .. Algorithm : DFS - pre-order algorithm
  */
  #define AstNodeEachStart(_root_, _stack_) do {       \
    _stack_[ 0 ] = NULL;                               \
    _stack_[ 1 ] = _root_->child;                      \
    int _l_ = 1;                                       \
    while ( _l_ ) {                                    \
      while ( *_stack_[ _l_ ] ) {                      \
        _AstNode * _node_ = *_stack_[ _l_ ];           \
        _AstNode * node = _node_;                      \
        /* Do something with node */     

  #define AstNodeEachEnd(_stack_)                      \
        /* Go right. .. But only                       \
        .. After covering all it's children */         \
        (_stack_[ _l_ ])++;                            \
        if ( _node_->child ) {                         \
          _stack_[ _l_+1 ] = _node_->child;            \
          /* if it fails rerun with larger stack size*/\
          assert( _l_ < _H_AST_STACK_SIZE_ - 1 );      \
          /* Go down */                                \
          ++_l_;                                       \
        }                                              \
      }                                                \
                                                       \
      /* Go up */                                      \
      --_l_;                                           \
    }                                                  \
  } while (0) ;

  /*
  .. Identifer type. Encode this to hash->attr
  */
  #define AST_UNKNOWN    NULL
  #define AST_IDENTIFIER ((void *) -1)
  #define AST_TYPEDEF    ((void *) -2)
  #define AST_ENUM       ((void *) -3)
  
  /*
  .. (a) initialize an AST. parameter is source code name
  .. (b) reset source location 
  .. (c) create a terminal node
  .. (d) create an internal node
  .. (e) set children of an internal node.
  .. (f) print ast back to C code. 
  */
  extern _Ast *     ast_init ( const char * );
  extern void       ast_reset_source ( _Ast * , const char * );
  extern _AstNode * ast_tnode_new ( _Ast *, int, const char * );
  extern _AstNode * ast_node_new (_Ast *, int, int );
  extern void       ast_node_children (_AstNode *, int, ... );
  extern void       ast_print (_Ast * ast);
  
#endif
