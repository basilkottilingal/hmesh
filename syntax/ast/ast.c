#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ast.h>

/*
.. Create an AST Node
*/
 
_AstTNode * 
ast_tnode_new (_AstNode * parent, int symbol) {

  _AstTNode 
 
  *node = (_AstNode) { 
    .symbol  = symbol,
    .child   = NULL, 
    .parent  = parent,
    .right   = NULL
  };

  return node;  
}

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
  size_t len = end - s; 
  if(len >= _H_AST_FILENAME_MAX_) {
    fprintf(stderr, 
      "ast_reset_source() parser error : "
      "very large filename : %s\n"
      "Rerun the program with larger filename buffer.\n"
      "Ex: -D_H_AST_FILENAME_MAX_=4096",
      s-1);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  memcpy(loc->source, s, len);
  loc->source[len] = '\0';

}

/* 
.. Initialize, start creating a tree with a root node
.. and maintain other global states.
*/
_Ast * ast_init(const char * source) {

  _Ast * ast = (_Ast *) malloc (sizeof(_Ast));
  _AstLoc * loc = &ast->loc;
  loc->line = loc->column = 1;
  size_t len = strlen ( source );
  if(len >= _H_AST_FILENAME_MAX_) {
    fprintf(stderr, 
      "ast_init() parser error : "
      "very large source name : \"%s\"\n."
      "Rerun the program with larger filename buffer.\n"
      "Ex: -D_H_AST_FILENAME_MAX_=4096",
      source);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  memcpy(loc->source, source, len+1);

  ast->root = (_AstNode) {
    .symbol = -1,
    .child  = NULL, 
    .parent = NULL,
    .right  = NULL
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
