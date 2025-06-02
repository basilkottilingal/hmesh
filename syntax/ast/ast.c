#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <ast.h>
#include <memory.h>
#include <hash.h>

/*
.. Create an AST Node
*/
 

/*
.. Reset source file name, whenever
..   # lineno "file.h"
.. detected
*/
void ast_reset_source(_Ast * ast, const char * str) {
  char * s    = strchr (str, '#') + 1;
  _AstLoc * loc = &ast->loc;
  loc->line   = atoi(s) - 1;
  loc->column = 1; 
  
  s = strchr (str, '"') + 1;
  char * end = strchr(s, '"');
  *end = '\0';
  loc->source = ast_strdup (s); 
  if(!loc->source) {
    fprintf(stderr, 
      "ast_reset_source() parser error : "
      "(possibly) very large filename : %s\n", s-1);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  *end = '"';
}

/* 
.. Initialize, start creating a tree with a root node
.. and maintain other global states.
*/
_Ast * ast_init(const char * source) {

  _Ast * ast = ast_allocate_general ( sizeof(_Ast) );
  _AstLoc * loc = &ast->loc;
  loc->line = loc->column = 1;
  loc->source = ast_strdup (source); 
  if(!loc->source) {
    fprintf(stderr, 
      "ast_reset_source() parser error : "
      "(possibly) very large filename : \"%s\"\n", source);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  ast->root = (_AstNode) {
    .symbol = -1,
    .child  = ast_allocate_general ( 2 * sizeof(_AstNode) ), 
    .parent = NULL
  };

  ast->nodes = ast_pool ( sizeof (_AstNode) );
  ast->tnodes = ast_pool ( sizeof (_AstTNode) );
  ast->identifiers = hash_table_init ();

  return ast;
}

void ast_free ( _Ast * ast ) {
  hash_table_free ( ast->identifiers );
  ast_deallocate_all ();
}

/*
.. Create a terminal node.
*/
_AstNode * 
ast_tnode_new (_Ast * ast, int symbol, const char * token) {

  _AstTNode * tnode = ast_allocate_from (ast->tnodes);

  _AstNode * node = &tnode->node;
  node->symbol = symbol;
  node->parent = NULL;
  node->child = NULL;
  if ( token ) {
    tnode->token = ast_strdup (token) ;
    /*
    .. warning : will fail for very large token, 
    .. like strings longer than 4k Bytes.
    */
    assert ( tnode->token );
  }

  memcpy ( &tnode->loc, &ast->loc, sizeof (_AstLoc) );

  return node;  
}

_AstNode *
ast_node_new (_Ast * ast, int symbol, int n) {
  _AstNode * node = ast_allocate_from (ast->nodes);
  node->symbol = symbol;
  node->parent = NULL;
  node->child = ast_allocate_general ( (n+1) * sizeof (_AstNode*) );
  return node;
}

void
ast_node_children (_AstNode * node, int n, ... ) {
  va_list args;      
  va_start(args, n);
  _AstNode * child = NULL;
  for (int i=0; i<n; ++i) {
    child = va_arg (args, _AstNode *);
    child->parent = node;
    node->child[i] = child; 
  }
  va_end(args);      
}

_AstNode *** _AST_STACK_ = NULL;

static inline
_AstNode *** ast_stack () {
  if(!_AST_STACK_)
    _AST_STACK_ = 
      ast_allocate_general (_H_AST_STACK_SIZE_ * sizeof(_AstNode **));
  return _AST_STACK_;
}

void
ast_print (_Ast * ast) {

 _AstNode *** stack = ast_stack();
  if (!stack) {
    fprintf (stderr, "ast_print() stack not available");
    fflush(stderr);
    exit (EXIT_FAILURE);
  }
  
  const char * source = NULL;

  const char * sp = NULL, * in [] = 
    { "\n", "\n  ", "\n    ", "\n      ", "\n        ", " ", ""};          
 
  int indent = 0, toggle = 1;

  AstNodeEachStart (ast, stack) 
    if (!node->child) {

      _AstTNode * t = (_AstTNode *) node;
      
      if (source != t->loc.source) {
        source = t->loc.source;
        printf("\n#line %d \"%s\"\n", t->loc.line, source);
        toggle = 0;
      }

      assert (t->token);
      sp = toggle ? in[indent % 5] : in[5];
      switch ( t->token[0] ) {
        case '{' : 
          toggle = 1; 
          ++indent; 
          break;
        case '}' : 
          toggle = 1; 
          --indent; 
          sp = in[indent % 5];
          break;
        case ';' : 
          toggle = 1; 
          break;
        case  '.':
          sp = in[6];
          break;
        default :
          /* fixme : use YYSYMBOL_ELSE */
          if(!strcmp ("else", t->token))
            sp = in [indent%5];
          toggle = 0;
      }
      printf ("%s%s", sp, t->token);
    }
  AstNodeEachEnd (ast, stack) 
}
