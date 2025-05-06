# ifndef _HMESH_AST_
# define _HMESH_AST_
  
  
    
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
    FILE * file;
  } _AstLoc;

  /* 
  .. Macro that update ast->loc (i.e parser location) when a character
  .. is parsed.
  .. Assumes tab width = 8.
  .. Doesn't handle other non-common escape keys.
  */
  #define AstLocInput( __c__ )                         \
    do {                                               \
      if ( (__c__) == '\n' ) {                         \
        ++(ast->loc.line);                             \
        ast->loc.column = 1;                           \
      }                                                \
      else if ( (__c__) == '\t' ) {                    \
        ast->loc.column += 8 - (ast->loc.column)%8;    \
      }                                                \
      else {                                           \
        ++(ast->loc.column);                           \
      }                                                \
    } while ( 0 ) ;

  /*
  .. Macro that updates parser loc after going through a full token.
  .. A small overhead, especially for longer token. O (n). 
  .. Limit to minimal case.
  */
  #define AstLocRead( __text__ )   \
    do {                           \
      char * c = __text__;         \
      while (*c) {                 \
        AstLocInput ( c );         \
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
  ..   @ loc      : locate token in file @ line/col
  ..   @ symbol   : token/semantic rule id
  */
  typedef struct _AstNode {
    struct _AstNode * child, * sibling, * parent;
    unsigned char type;
    _AstLoc loc;
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
