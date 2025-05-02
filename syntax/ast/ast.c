#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ast.h>

/*
.. Create an AST Node
*/
 
_Ast * 
AstNode(_Ast * parent, int symbol, int lineno, unsigned char type) {
  /* fixme : Use mempool */
  _Ast * node = (_Ast *) malloc (sizeof(_Ast));
 
  *node = (_Ast) { 
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
void AstResetSource(_AstGlobal * ast, const char * source) {

  if (strlen(source) >= _HMESH_PARSER_FILENAME_MAX_) {
    fprintf(stderr, "AstResetSource() : very large filename : %s\n"
      "Rerun the program with -D_HMESH_PARSER_FILENAME_MAX_=4096",
      source);
    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  strcpy(ast->source, source);
  
}

/* 
.. Keep track of ast environment
*/
_AstGlobal * AstInit(const char * source) {

  _AstGlobal * ast = (_AstGlobal *) malloc (sizeof(_AstGlobal));
  AstResetSource(ast, source);
  ast->root = AstNode (NULL, -1, 0, AstNodeIsInternal);

  return ast;
}

