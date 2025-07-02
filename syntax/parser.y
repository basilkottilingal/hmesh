/*
.. ANSI C Yacc grammar
.. (This Yacc file is accompanied by a matching Lex file.)
.. 
.. In 1985, Jeff Lee published his Yacc grammar based on a draft version
.. of the ANSI C standard, along with a supporting Lex specification. Tom
.. Stockfisch reposted those files to net.sources in 1987; as mentioned
.. in the answer to question 17.25 of the comp.lang.c FAQ, they used to
.. be available from ftp.uu.net as usenet/net.sources/ansi.c.grammar.Z.
.. 
.. The version you see here has been updated based on the 2011 ISO C
.. standard. (The previous version's Lex and Yacc files for ANSI C9X
.. still exist as archived copies.)
.. 
.. This grammar assumes that translation phases 1..5 have already been
.. completed, including preprocessing and _Pragma processing. The Lex
.. rule for string literals will perform concatenation (translation phase
.. 6). Transliteration of universal character names (\uHHHH or
.. \UHHHHHHHH) must have been done by either the preprocessor or a
.. replacement for the input() macro used by Lex (or the YY_INPUT
.. function used by Flex) to read characters. Although comments should
.. have been changed to space characters during translation phase 3,
.. there are Lex rules for them anyway.
.. 
.. I want to keep this version as close to the current C Standard grammar
.. as possible; please let me know if you discover discrepancies.  (There
.. is an FAQ for this grammar that you might want to read first.)
.. 
.. jutta@pobox.com, 2012
.. 
.. Last edit: 2012-12-18 DAGwyn@aol.com
.. Note: There are two shift/reduce conflicts, correctly resolved by default:
.. 
..   IF '(' expression ')' statement _ ELSE statement
.. 
.. and
.. 
..   ATOMIC _ '(' type_name ')'
.. 
.. where "_" has been used to flag the points of ambiguity.
.. */

/* 
.. Set the output as 'parser.c' 
*/
%output  "parser.c"
%defines "parser.h"

/* 
.. Fully pure reentrant parser, 
.. Semantic value type as _AstNode *,
.. add parameter 
*/
%define api.pure full
%param {_Ast * ast}
%define api.value.type {_AstNode *}

%{
  
  #include <string.h>
  #include <assert.h>
  #include <stdlib.h>
  #include <stdio.h>
  
  #include <ast.h>
  
  /* 
  .. Following extern variables are defined in lexer.c and
  .. they are respectively the default names used by flex 
  .. for input stream and lineno.
  */
  extern FILE *yyin;  
  
  /* 
  .. Forward declaration of yylex(), which is the main lexer 
  .. function defined by flex 
  */
  int yylex( _AstNode ** node, _Ast * ast );
  
  /* 
  .. Forward declaration of yyparse(), which is the main parser 
  .. function defined by bison 
  */
  int yyparse( _Ast * ast );
  
  /* 
  .. Error function 
  */
  void yyerror( _Ast * ast,  const char *s);
%}

%token  IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token  PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token  AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token  SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token  XOR_ASSIGN OR_ASSIGN
%token  TYPEDEF_NAME ENUMERATION_CONSTANT

%token  TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token  CONST RESTRICT VOLATILE
%token  BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token  COMPLEX IMAGINARY 
%token  STRUCT UNION ENUM ELLIPSIS

%token  CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token  ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%token  SEMICOLON LBRACE RBRACE COMMA COLON EQUAL LPARENTHESIS RPARENTHESIS
%token  LBRACKET RBRACKET DOT AMPERSAND NOT TILDE MINUS PLUS STAR
%token  SLASH PERCENT L_T G_T CARET PIPE QUESTION

%start  root
%%

primary_expression /*ti*/
  : /*>>*/  IDENTIFIER
  |  constant
  |  string
  |  LPARENTHESIS /*>*/  expression /*>>*/  RPARENTHESIS
  | /*>*/  generic_selection
  ;

constant /**/
  :  I_CONSTANT
  |  F_CONSTANT
  |  ENUMERATION_CONSTANT
  ;

string /**/
  :  STRING_LITERAL
  |  FUNC_NAME
  ;

expression /*ti*/
  : /*>*/  assignment_expression
  | /*>*/  expression  COMMA  assignment_expression
  ;

generic_selection /*ti*/
  :  GENERIC  LPARENTHESIS /*>*/  assignment_expression  COMMA  generic_assoc_list /*>>*/  RPARENTHESIS
  ;

enumeration_constant /*i*/
  : /*>>*/  IDENTIFIER
  ;

assignment_expression /*ti*/
  : /*>*/  conditional_expression
  | /*>*/  unary_expression  assignment_operator  assignment_expression
  ;

generic_assoc_list /*ti*/
  : /*>*/  generic_association
  | /*>*/  generic_assoc_list  COMMA  generic_association
  ;

generic_association /*ti*/
  : /*>*/  type_name  COLON  assignment_expression
  |  DEFAULT  COLON /*>*/  assignment_expression
  ;

type_name /*ti*/
  : /*>*/  specifier_qualifier_list  abstract_declarator
  | /*>*/  specifier_qualifier_list
  ;

postfix_expression /*ti*/
  : /*>*/  primary_expression
  | /*>*/  postfix_expression  LBRACKET  expression /*>>*/  RBRACKET
  | /*>*/  postfix_expression /*>>*/  LPARENTHESIS  RPARENTHESIS
  | /*>*/  postfix_expression  LPARENTHESIS  argument_expression_list /*>>*/  RPARENTHESIS
  | /*>*/  postfix_expression /*>>*/  DOT  IDENTIFIER
  | /*>*/  postfix_expression /*>>*/  PTR_OP  IDENTIFIER
  | /*>*/  postfix_expression /*>>*/  INC_OP
  | /*>*/  postfix_expression /*>>*/  DEC_OP
  |  LPARENTHESIS /*>*/  type_name  RPARENTHESIS  LBRACE  initializer_list /*>>*/  RBRACE
  |  LPARENTHESIS /*>*/  type_name  RPARENTHESIS  LBRACE  initializer_list /*>>*/  COMMA  RBRACE
  ;

argument_expression_list /*ti*/
  : /*>*/  assignment_expression
  | /*>*/  argument_expression_list  COMMA  assignment_expression
  ;

initializer_list /*ti*/
  : /*>*/  designation  initializer
  | /*>*/  initializer
  | /*>*/  initializer_list  COMMA  designation  initializer
  | /*>*/  initializer_list  COMMA  initializer
  ;

unary_expression /*ti*/
  : /*>*/  postfix_expression
  |  INC_OP /*>*/  unary_expression
  |  DEC_OP /*>*/  unary_expression
  |  unary_operator /*>*/  cast_expression
  |  SIZEOF /*>*/  unary_expression
  |  SIZEOF  LPARENTHESIS /*>*/  type_name /*>>*/  RPARENTHESIS
  |  ALIGNOF  LPARENTHESIS /*>*/  type_name /*>>*/  RPARENTHESIS
  ;

unary_operator /**/
  :  AMPERSAND
  |  STAR
  |  PLUS
  |  MINUS
  |  TILDE
  |  NOT
  ;

cast_expression /*ti*/
  : /*>*/  unary_expression
  |  LPARENTHESIS /*>*/  type_name  RPARENTHESIS  cast_expression
  ;

multiplicative_expression /*ti*/
  : /*>*/  cast_expression
  | /*>*/  multiplicative_expression  STAR  cast_expression
  | /*>*/  multiplicative_expression  SLASH  cast_expression
  | /*>*/  multiplicative_expression  PERCENT  cast_expression
  ;

additive_expression /*ti*/
  : /*>*/  multiplicative_expression
  | /*>*/  additive_expression  PLUS  multiplicative_expression
  | /*>*/  additive_expression  MINUS  multiplicative_expression
  ;

shift_expression /*ti*/
  : /*>*/  additive_expression
  | /*>*/  shift_expression  LEFT_OP  additive_expression
  | /*>*/  shift_expression  RIGHT_OP  additive_expression
  ;

relational_expression /*ti*/
  : /*>*/  shift_expression
  | /*>*/  relational_expression  L_T  shift_expression
  | /*>*/  relational_expression  G_T  shift_expression
  | /*>*/  relational_expression  LE_OP  shift_expression
  | /*>*/  relational_expression  GE_OP  shift_expression
  ;

equality_expression /*ti*/
  : /*>*/  relational_expression
  | /*>*/  equality_expression  EQ_OP  relational_expression
  | /*>*/  equality_expression  NE_OP  relational_expression
  ;

and_expression /*ti*/
  : /*>*/  equality_expression
  | /*>*/  and_expression  AMPERSAND  equality_expression
  ;

exclusive_or_expression /*ti*/
  : /*>*/  and_expression
  | /*>*/  exclusive_or_expression  CARET  and_expression
  ;

inclusive_or_expression /*ti*/
  : /*>*/  exclusive_or_expression
  | /*>*/  inclusive_or_expression  PIPE  exclusive_or_expression
  ;

logical_and_expression /*ti*/
  : /*>*/  inclusive_or_expression
  | /*>*/  logical_and_expression  AND_OP  inclusive_or_expression
  ;

logical_or_expression /*ti*/
  : /*>*/  logical_and_expression
  | /*>*/  logical_or_expression  OR_OP  logical_and_expression
  ;

conditional_expression /*ti*/
  : /*>*/  logical_or_expression
  | /*>*/  logical_or_expression  QUESTION  expression  COLON  conditional_expression
  ;

assignment_operator /**/
  :  EQUAL
  |  MUL_ASSIGN
  |  DIV_ASSIGN
  |  MOD_ASSIGN
  |  ADD_ASSIGN
  |  SUB_ASSIGN
  |  LEFT_ASSIGN
  |  RIGHT_ASSIGN
  |  AND_ASSIGN
  |  XOR_ASSIGN
  |  OR_ASSIGN
  ;

constant_expression /*ti*/
  : /*>*/  conditional_expression
  ;

declaration /*ti*/
  : /*>*/  declaration_specifiers /*>>*/  SEMICOLON
  | /*>*/  declaration_specifiers  init_declarator_list /*>>*/  SEMICOLON
  | /*>*/  static_assert_declaration
  ;

declaration_specifiers /*ti*/
  :  storage_class_specifier /*>*/  declaration_specifiers
  |  storage_class_specifier
  | /*>*/  type_specifier  declaration_specifiers
  | /*>*/  type_specifier
  |  type_qualifier /*>*/  declaration_specifiers
  |  type_qualifier
  |  function_specifier /*>*/  declaration_specifiers
  |  function_specifier
  | /*>*/  alignment_specifier  declaration_specifiers
  | /*>*/  alignment_specifier
  ;

init_declarator_list /*ti*/
  : /*>*/  init_declarator
  | /*>*/  init_declarator_list  COMMA  init_declarator
  ;

static_assert_declaration /*ti*/
  :  STATIC_ASSERT  LPARENTHESIS /*>*/  constant_expression /*>>*/  COMMA  STRING_LITERAL  RPARENTHESIS  SEMICOLON
  ;

storage_class_specifier /**/
  :  TYPEDEF
  |  EXTERN
  |  STATIC
  |  THREAD_LOCAL
  |  AUTO
  |  REGISTER
  ;

type_specifier /*ti*/
  :  VOID
  |  CHAR
  |  SHORT
  |  INT
  |  LONG
  |  FLOAT
  |  DOUBLE
  |  SIGNED
  |  UNSIGNED
  |  BOOL
  |  COMPLEX
  |  IMAGINARY
  | /*>*/  atomic_type_specifier
  | /*>*/  struct_or_union_specifier
  | /*>*/  enum_specifier
  |  TYPEDEF_NAME
  ;

type_qualifier /**/
  :  CONST
  |  RESTRICT
  |  VOLATILE
  |  ATOMIC
  ;

function_specifier /**/
  :  INLINE
  |  NORETURN
  ;

alignment_specifier /*ti*/
  :  ALIGNAS  LPARENTHESIS /*>*/  type_name /*>>*/  RPARENTHESIS
  |  ALIGNAS  LPARENTHESIS /*>*/  constant_expression /*>>*/  RPARENTHESIS
  ;

init_declarator /*ti*/
  : /*>*/  declarator  EQUAL  initializer
  | /*>*/  declarator
  ;

declarator /*ti*/
  :  pointer /*>*/  direct_declarator
  | /*>*/  direct_declarator
  ;

initializer /*ti*/
  :  LBRACE /*>*/  initializer_list /*>>*/  RBRACE
  |  LBRACE /*>*/  initializer_list /*>>*/  COMMA  RBRACE
  | /*>*/  assignment_expression
  ;

atomic_type_specifier /*ti*/
  :  ATOMIC  LPARENTHESIS /*>*/  type_name /*>>*/  RPARENTHESIS
  ;

struct_or_union_specifier /*ti*/
  :  struct_or_union  LBRACE /*>*/  struct_declaration_list /*>>*/  RBRACE
  |  struct_or_union /*>*/  IDENTIFIER  LBRACE  struct_declaration_list /*>>*/  RBRACE
  |  struct_or_union /*>>*/  IDENTIFIER
  ;

enum_specifier /*ti*/
  :  ENUM  LBRACE /*>*/  enumerator_list /*>>*/  RBRACE
  |  ENUM  LBRACE /*>*/  enumerator_list /*>>*/  COMMA  RBRACE
  |  ENUM /*>*/  IDENTIFIER  LBRACE  enumerator_list /*>>*/  RBRACE
  |  ENUM /*>*/  IDENTIFIER  LBRACE  enumerator_list /*>>*/  COMMA  RBRACE
  |  ENUM /*>>*/  IDENTIFIER
  ;

struct_or_union /**/
  :  STRUCT
  |  UNION
  ;

struct_declaration_list /*ti*/
  : /*>*/  struct_declaration
  | /*>*/  struct_declaration_list  struct_declaration
  ;

struct_declaration /*ti*/
  : /*>*/  specifier_qualifier_list /*>>*/  SEMICOLON
  | /*>*/  specifier_qualifier_list  struct_declarator_list /*>>*/  SEMICOLON
  | /*>*/  static_assert_declaration
  ;

specifier_qualifier_list /*ti*/
  : /*>*/  type_specifier  specifier_qualifier_list
  | /*>*/  type_specifier
  |  type_qualifier /*>*/  specifier_qualifier_list
  |  type_qualifier
  ;

struct_declarator_list /*ti*/
  : /*>*/  struct_declarator
  | /*>*/  struct_declarator_list  COMMA  struct_declarator
  ;

struct_declarator /*ti*/
  :  COLON /*>*/  constant_expression
  | /*>*/  declarator  COLON  constant_expression
  | /*>*/  declarator
  ;

enumerator_list /*ti*/
  : /*>*/  enumerator
  | /*>*/  enumerator_list  COMMA  enumerator
  ;

enumerator /*ti*/
  : /*>*/  enumeration_constant  EQUAL  constant_expression
  | /*>>*/  enumeration_constant
  ;

pointer /**/
  :  STAR  type_qualifier_list  pointer
  |  STAR  type_qualifier_list
  |  STAR  pointer
  |  STAR
  ;

direct_declarator /*ti*/
  : /*>>*/  IDENTIFIER
  |  LPARENTHESIS /*>*/  declarator /*>>*/  RPARENTHESIS
  | /*>*/  direct_declarator /*>>*/  LBRACKET  RBRACKET
  | /*>*/  direct_declarator /*>>*/  LBRACKET  STAR  RBRACKET
  | /*>*/  direct_declarator  LBRACKET  STATIC  type_qualifier_list  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_declarator  LBRACKET  STATIC  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_declarator /*>>*/  LBRACKET  type_qualifier_list  STAR  RBRACKET
  | /*>*/  direct_declarator  LBRACKET  type_qualifier_list  STATIC  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_declarator  LBRACKET  type_qualifier_list  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_declarator /*>>*/  LBRACKET  type_qualifier_list  RBRACKET
  | /*>*/  direct_declarator  LBRACKET  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_declarator  LPARENTHESIS  parameter_type_list /*>>*/  RPARENTHESIS
  | /*>*/  direct_declarator /*>>*/  LPARENTHESIS  RPARENTHESIS
  | /*>*/  direct_declarator /*>>*/  LPARENTHESIS  identifier_list  RPARENTHESIS
  ;

type_qualifier_list /**/
  :  type_qualifier
  |  type_qualifier_list  type_qualifier
  ;

parameter_type_list /*ti*/
  : /*>*/  parameter_list /*>>*/  COMMA  ELLIPSIS
  | /*>*/  parameter_list
  ;

identifier_list /*i*/
  : /*>>*/  IDENTIFIER
  | /*>>*/  identifier_list  COMMA  IDENTIFIER
  ;

parameter_list /*ti*/
  : /*>*/  parameter_declaration
  | /*>*/  parameter_list  COMMA  parameter_declaration
  ;

parameter_declaration /*ti*/
  : /*>*/  declaration_specifiers  declarator
  | /*>*/  declaration_specifiers  abstract_declarator
  | /*>*/  declaration_specifiers
  ;

abstract_declarator /*ti*/
  :  pointer /*>*/  direct_abstract_declarator
  |  pointer
  | /*>*/  direct_abstract_declarator
  ;

direct_abstract_declarator /*ti*/
  :  LPARENTHESIS /*>*/  abstract_declarator /*>>*/  RPARENTHESIS
  |  LBRACKET  RBRACKET
  |  LBRACKET  STAR  RBRACKET
  |  LBRACKET  STATIC  type_qualifier_list /*>*/  assignment_expression /*>>*/  RBRACKET
  |  LBRACKET  STATIC /*>*/  assignment_expression /*>>*/  RBRACKET
  |  LBRACKET  type_qualifier_list  STATIC /*>*/  assignment_expression /*>>*/  RBRACKET
  |  LBRACKET  type_qualifier_list /*>*/  assignment_expression /*>>*/  RBRACKET
  |  LBRACKET  type_qualifier_list  RBRACKET
  |  LBRACKET /*>*/  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_abstract_declarator /*>>*/  LBRACKET  RBRACKET
  | /*>*/  direct_abstract_declarator /*>>*/  LBRACKET  STAR  RBRACKET
  | /*>*/  direct_abstract_declarator  LBRACKET  STATIC  type_qualifier_list  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_abstract_declarator  LBRACKET  STATIC  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_abstract_declarator  LBRACKET  type_qualifier_list  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_abstract_declarator  LBRACKET  type_qualifier_list  STATIC  assignment_expression /*>>*/  RBRACKET
  | /*>*/  direct_abstract_declarator /*>>*/  LBRACKET  type_qualifier_list  RBRACKET
  | /*>*/  direct_abstract_declarator  LBRACKET  assignment_expression /*>>*/  RBRACKET
  |  LPARENTHESIS  RPARENTHESIS
  |  LPARENTHESIS /*>*/  parameter_type_list /*>>*/  RPARENTHESIS
  | /*>*/  direct_abstract_declarator /*>>*/  LPARENTHESIS  RPARENTHESIS
  | /*>*/  direct_abstract_declarator  LPARENTHESIS  parameter_type_list /*>>*/  RPARENTHESIS
  ;

designation /*ti*/
  : /*>*/  designator_list /*>>*/  EQUAL
  ;

designator_list /*ti*/
  : /*>*/  designator
  | /*>*/  designator_list  designator
  ;

designator /*ti*/
  :  LBRACKET /*>*/  constant_expression /*>>*/  RBRACKET
  |  DOT /*>>*/  IDENTIFIER
  ;

statement /*ti*/
  : /*>*/  labeled_statement
  | /*>*/  compound_statement
  | /*>*/  expression_statement
  | /*>*/  selection_statement
  | /*>*/  iteration_statement
  | /*>*/  jump_statement
  ;

labeled_statement /*ti*/
  : /*>*/  IDENTIFIER  COLON  statement
  |  CASE /*>*/  constant_expression  COLON  statement
  |  DEFAULT  COLON /*>*/  statement
  ;

compound_statement /*ti*/
  :  LBRACE  RBRACE
  |  LBRACE /*>*/  block_item_list /*>>*/  RBRACE
  ;

expression_statement /*ti*/
  :  SEMICOLON
  | /*>*/  expression /*>>*/  SEMICOLON
  ;

selection_statement /*ti*/
  :  IF  LPARENTHESIS /*>*/  expression  RPARENTHESIS  statement  ELSE  statement
  |  IF  LPARENTHESIS /*>*/  expression  RPARENTHESIS  statement
  |  SWITCH  LPARENTHESIS /*>*/  expression  RPARENTHESIS  statement
  ;

iteration_statement /*ti*/
  :  WHILE  LPARENTHESIS /*>*/  expression  RPARENTHESIS  statement
  |  DO /*>*/  statement  WHILE  LPARENTHESIS  expression /*>>*/  RPARENTHESIS  SEMICOLON
  |  FOR  LPARENTHESIS /*>*/  expression_statement  expression_statement  RPARENTHESIS  statement
  |  FOR  LPARENTHESIS /*>*/  expression_statement  expression_statement  expression  RPARENTHESIS  statement
  |  FOR  LPARENTHESIS /*>*/  declaration  expression_statement  RPARENTHESIS  statement
  |  FOR  LPARENTHESIS /*>*/  declaration  expression_statement  expression  RPARENTHESIS  statement
  ;

jump_statement /*ti*/
  :  GOTO /*>>*/  IDENTIFIER  SEMICOLON
  |  CONTINUE  SEMICOLON
  |  BREAK  SEMICOLON
  |  RETURN  SEMICOLON
  |  RETURN /*>*/  expression /*>>*/  SEMICOLON
  ;

block_item_list /*ti*/
  : /*>*/  block_item
  | /*>*/  block_item_list  block_item
  ;

block_item /*ti*/
  : /*>*/  declaration
  | /*>*/  statement
  ;

root /*ti*/
  : /*>*/  translation_unit /*>>*/  { 
      $$ = &ast->root;
      $$->symbol = YYSYMBOL_root;
      $$->child[0] = $1;
      $1->parent = $$;
    }
  ;

translation_unit /*ti*/
  : /*>*/  external_declaration
  | /*>*/  translation_unit  external_declaration
  ;

external_declaration /*ti*/
  : /*>*/  function_definition
  | /*>*/  declaration
  ;

function_definition /*ti*/
  : /*>*/  declaration_specifiers  declarator  declaration_list  compound_statement
  | /*>*/  declaration_specifiers  declarator  compound_statement
  ;

declaration_list /*ti*/
  : /*>*/  declaration
  | /*>*/  declaration_list  declaration
  ;

%%
  
  #include <stdio.h>
  
  void yyerror( _Ast * ast, const char * msg)
  {
    /*
    .. prints error message, with source code location
    ..    filename:lineno:column
    */ 
    fflush(stdout);
    fprintf(stderr, "*** syntax error in file %s:%d:%d\n*** %s\n", 
      ast->loc.source, ast->loc.line, ast->loc.column - 1, msg);
  }
  
  int main(int argc, char ** argv) {
     
    /* Expects second arguement as the input file (i.e the source code)
    .. to be parsed */ 
    if (argc < 2) {
      fprintf(stderr, "Usage: %s input_file\n", argv[0]);
      return 1;
    }
  
    /* Open input file */
    yyin = fopen(argv[1], "r");
    if (!yyin) {
      perror("Error opening input file");
      return 1;
    }
  
    /* 
    .. Initializing.
    .. Set up the tree (with root node) 
    */
    _Ast * ast = ast_init (argv[1]);
      
    /* 
    .. Parse all tokens and build AST in the process
    */
    int status = yyparse(ast);
    (void) status;

    /*
    .. fixme: use status, to see if parser has exited properly
    */
    ast_print (ast, NULL);

    /*
    .. AST syntax check, analysis, etc
    */
    return 0;
  }
