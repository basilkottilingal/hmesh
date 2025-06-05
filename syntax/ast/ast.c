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

  ast->scope = 0;
  ast->scopemax = 2;
  ast->symbols = malloc ( 2 * sizeof (_HashTable *) );
  
  /* 
  .. 1024 and 128 is the size of sym table @ scope 0 & 1 resp.
  */
  ast->symbols[0] = hash_table_init ( 10 ); 
  ast->symbols[1] = hash_table_init ( 7 );

  return ast;
}

void ast_free ( _Ast * ast ) {
  for(int i=0; i<ast->scopemax; ++i)
    hash_table_free ( ast->symbols[i] );
  free (ast->symbols);
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
ast_print (_Ast * ast, _AstNode * Root) {

 _AstNode *** stack = ast_stack();
  if (!stack) {
    fprintf (stderr, "ast_print() stack not available");
    fflush(stderr);
    exit (EXIT_FAILURE);
  }
  
  const char * source = NULL;
  int line = 0, column = 0;
  _AstLoc * loc;

  _AstNode * root = Root ? Root : &ast->root;

  if(!root->child)
    /* 
    .. There is nothing to print.
    */
    return;

  AstNodeEachStart (root, stack) 
    if (!node->child) {

      _AstTNode * t = (_AstTNode *) node;
      char * token = t->token;
      assert(token);
     
      loc = &t->loc;
      if (source != t->loc.source) {
        source = loc->source;
        line   = loc->line;
        column = 1;
        printf("\n#line %d \"%s\"\n", loc->line, source);
      }

      if (line < loc->line) { 
        while (line < loc->line) {
          line++;
          putchar('\n');
        }
        column = 1;
      }

      while (column < loc->column) {
        column++;
        putchar(' ');
      }
      int c;
      while ( (c = (int) *token++) != '\0'){
        putchar(c);
        if(*token == '\n') {
          line++;
          column = 1;
        }
        else
          column++;
      }
    }
  AstNodeEachEnd (stack) 
}

/*
.. Retag an identifer as TYPEDEF_NAME or ENUMERATION_CONSTANT 
void ast_identifier_type (_Ast * ast, _AstNode * node, void * type) {
  assert(!node->child);
  _AstTNode * t = (_AstTNode *) node;
  assert(t->token);
  _HashNode * h = hash_lookup (ast->identifiers, t->token);
  assert(h);
  if ( h->attr != AST_IDENTIFIER ) {
    fprintf(stderr, "parser error : name %s already exist", t->token);
    return;
  }
  h->attr = type;
}
*/
