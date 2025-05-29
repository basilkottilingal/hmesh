# ifndef _H_AST_
# define _H_AST_

  #include <memory.h>
  #include <hash.h>

  /* 
  .. NOTE: length of the name of source file /include header file
  .. are limited to 1024 characters (including the path and trailing '\0') 
  .. if not overridden like -D_H_AST_FILENAME_=4096
  */
  #ifndef _H_AST_FILENAME_MAX_
    #define _H_AST_FILENAME_MAX_ 1024
  #endif
    
  /*
  .. Datatype to store, file name and line & column number of the source.
  */
    
  typedef struct _AstLoc {
    char source[_H_AST_FILENAME_MAX_];
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
    int symbol;
    struct _AstNode * parent,
                    * left,
                    * right, 
                    * child;
  } _AstNode;

  typedef struct {
    _AstNode node;
    _AstLoc  loc;
    char *   token;
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
  .. (a) initialize an AST. parameter is source code name
  .. (b) reset source location 
  .. (c) create a terminal node
  .. (d) create an internal node 
  */
  extern _Ast *     ast_init ( const char * );
  extern void       ast_reset_source ( _Ast * , const char * );
  extern _AstNode * ast_tnode_new ( _Ast *, int, const char * );
  /*extern _AstNode * ast_node_new (_Ast *, ... );*/
  
#endif
