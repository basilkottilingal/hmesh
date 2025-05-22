#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ast.h>

/*
.. Create an AST Node
*/
 
_AstNode * 
ast_node (_AstNode * parent, int symbol, unsigned char type) {
  /* fixme : Use mempool */
  _AstNode * node = (_AstNode *) malloc (sizeof(_AstNode));
 
  *node = (_AstNode) { 
    .symbol  = symbol,
    .type    = type,
    .child   = NULL, 
    .parent  = parent,
    .sibling = NULL
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
  ast->loc.line   = atoi(s) - 1;
  ast->loc.column = 1; 
  
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
  memcpy(ast->source, s, len);
  ast->source[len] = '\0';

}

/* 
.. Initialize, start creating a tree with a root node
.. and maintain other global states.
*/
_Ast * ast_init(const char * source) {

  _Ast * ast = (_Ast *) malloc (sizeof(_Ast));
  ast->loc.line = ast->loc.column = 1;
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
  memcpy(ast->source, source, len+1);

  ast->root = ast_node (NULL, -1, AstNodeIsInternal);

  return ast;
}

