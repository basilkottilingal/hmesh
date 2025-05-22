# ifndef _H_AST_
# define _H_AST_

  /* 
  .. NOTE: length of the name of source file /include header file
  .. are limited to 1024 characters (including the path and trailing '\0') 
  .. if not overridden like -D_H_AST_FILENAME_=4096
  */
  #ifndef _H_AST_FILENAME_MAX_
    #define _H_AST_FILENAME_MAX_ 1024
  #endif
    
  /*
  .. @ _AstLoc : Parser tracker 
  ..   [ There is no implcit column number tracker 
  ..     like yylineno in flex. So instead of using 
  ..       %option yylineno
  ..     , we are using a user defined struct _AstLoc for 
  ..     for tracking location of parser
  ..   ]
  ..   
  ..   @ file : filename of source code 
  ..   @ line, column : location in the source code that you are reading
  */
    
  typedef struct _AstLoc {
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
  .. -------------------------------------
  .. --------- AST Node ------------------
  .. -------------------------------------
  */
  
  /*
  .. @AstNode : represent a node of the Abstract Syntaxt Tree (AST)
  ..   @ child, sibling, parent : tree node connection
  ..   @ type     : which type of tree node ?
  ..   @ symbol   : token/semantic rule id
  */
  typedef struct _AstNode {
    struct _AstNode * child, * sibling, * parent;
    int type;
    int symbol;
  } _AstNode;
  
  /* 
  ..  Identify attribute of an ast node 
  */
  enum {
    AstNodeIsLeaf =  1,
    AstNodeIsInternal = 2
  };
  
  
  /*
  .. -------------------------------------
  .. --------- AST -----------------------
  .. -------------------------------------
  */
  
  
  
  /* @ _Ast : stores metadata related to AST (root node), .. 
  ..   parser global variables. 
  ..   [ when you use "pure" parser i,e if you have set api.pure as full, 
  ..     you don't use any global variables. As an Alternative,
  ..     we use arguements that you pass to yyparse() &/ yylex().
  ..   ]
  ..   
  ..   @ root  : root node of ast
  ..   @ loc   : current parser location. source file + line + column
  ..   @ stack : stack of identifiers.
  */
  typedef struct _Ast {
    char source[_H_AST_FILENAME_MAX_];
    _AstNode * root;
    _AstLoc loc;
  } _Ast;
  
  
  extern _AstNode * ast_node ( _AstNode *, int , unsigned char );
  extern _Ast *     ast_init ( const char * );
  extern void       ast_reset_source ( _Ast * , const char * );
  
#endif
