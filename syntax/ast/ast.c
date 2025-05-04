#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ast.h>

/*
.. Create an AST Node
*/
 
_AstNode * 
AstNode(_AstNode * parent, int symbol, int lineno, unsigned char type) {
  /* fixme : Use mempool */
  _AstNode * node = (_AstNode *) malloc (sizeof(_AstNode));
 
  *node = (_AstNode) { 
    .symbol  = symbol,
    .lineno  = lineno,
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
void AstResetSource(_Ast * ast, const char * source) {
    
  char * s = strchr (str, '#') + 1;
    
  *lineno = atoi(s) - 1;
    
  s = strchr (str, '"') + 1;

  if (strlen(source) >= _HMESH_PARSER_FILENAME_MAX_) {
    fprintf(stderr, "Hmesh parser error.\n"
      "AstResetSource() : very large filename : %s\n"
      "Rerun the program with larger filename buffer.\n"
      "Ex: -D_HMESH_PARSER_FILENAME_MAX_=4096",
      source);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  strcpy(ast->source, source);
  
}

/* 
.. Initialize, start creating a tree with a root node
.. and maintain other global states.
*/
_Ast * AstInit(const char * source) {

  _Ast * ast = (_Ast *) malloc (sizeof(_Ast));
  AstResetSource(ast, source);
  ast->root = AstNode (NULL, -1, 0, AstNodeIsInternal);

  return ast;
}

