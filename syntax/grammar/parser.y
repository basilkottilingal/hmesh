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

%token	IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	CONST RESTRICT VOLATILE
%token	BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%token  SEMICOLON LBRACE RBRACE COMMA COLON EQUAL LPARANTHESIS RPARANTHESIS
%token  LBRACKET RBRACKET DOT AMPERSAND NOT TILDE MINUS PLUS STAR
%token  SLASH PERCENT L_T R_T CARET PIPE QUESTION

%start  root
%%
primary_expression
  : IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_primary_expression);
    }
  | constant {
      $$ = ast_node_new (YYSYMBOL_primary_expression);
    }
  | string {
      $$ = ast_node_new (YYSYMBOL_primary_expression);
    }
  | LPARANTHESIS  expression  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_primary_expression);
    }
  | generic_selection {
      $$ = ast_node_new (YYSYMBOL_primary_expression);
    }
  ;
constant
  : I_CONSTANT {
      $$ = ast_node_new (YYSYMBOL_constant);
    }
  | F_CONSTANT {
      $$ = ast_node_new (YYSYMBOL_constant);
    }
  | ENUMERATION_CONSTANT {
      $$ = ast_node_new (YYSYMBOL_constant);
    }
  ;
enumeration_constant
  : IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_enumeration_constant);
    }
  ;
string
  : STRING_LITERAL {
      $$ = ast_node_new (YYSYMBOL_string);
    }
  | FUNC_NAME {
      $$ = ast_node_new (YYSYMBOL_string);
    }
  ;
generic_selection
  : GENERIC  LPARANTHESIS  assignment_expression  COMMA  generic_assoc_list  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_generic_selection);
    }
  ;
generic_assoc_list
  : generic_association {
      $$ = ast_node_new (YYSYMBOL_generic_assoc_list);
    }
  | generic_assoc_list  COMMA  generic_association {
      $$ = ast_node_new (YYSYMBOL_generic_assoc_list);
    }
  ;
generic_association
  : type_name  COLON  assignment_expression {
      $$ = ast_node_new (YYSYMBOL_generic_association);
    }
  | DEFAULT  COLON  assignment_expression {
      $$ = ast_node_new (YYSYMBOL_generic_association);
    }
  ;
postfix_expression
  : primary_expression {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  LBRACKET  expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  LPARANTHESIS  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  LPARANTHESIS  argument_expression_list  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  DOT  IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  PTR_OP  IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  INC_OP {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | postfix_expression  DEC_OP {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | LPARANTHESIS  type_name  RPARANTHESIS  LBRACE  initializer_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  | LPARANTHESIS  type_name  RPARANTHESIS  LBRACE  initializer_list  COMMA  RBRACE {
      $$ = ast_node_new (YYSYMBOL_postfix_expression);
    }
  ;
argument_expression_list
  : assignment_expression {
      $$ = ast_node_new (YYSYMBOL_argument_expression_list);
    }
  | argument_expression_list  COMMA  assignment_expression {
      $$ = ast_node_new (YYSYMBOL_argument_expression_list);
    }
  ;
unary_expression
  : postfix_expression {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  | INC_OP  unary_expression {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  | DEC_OP  unary_expression {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  | unary_operator  cast_expression {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  | SIZEOF  unary_expression {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  | SIZEOF  LPARANTHESIS  type_name  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  | ALIGNOF  LPARANTHESIS  type_name  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_unary_expression);
    }
  ;
unary_operator
  : AMPERSAND {
      $$ = ast_node_new (YYSYMBOL_unary_operator);
    }
  | STAR {
      $$ = ast_node_new (YYSYMBOL_unary_operator);
    }
  | PLUS {
      $$ = ast_node_new (YYSYMBOL_unary_operator);
    }
  | MINUS {
      $$ = ast_node_new (YYSYMBOL_unary_operator);
    }
  | TILDE {
      $$ = ast_node_new (YYSYMBOL_unary_operator);
    }
  | NOT {
      $$ = ast_node_new (YYSYMBOL_unary_operator);
    }
  ;
cast_expression
  : unary_expression {
      $$ = ast_node_new (YYSYMBOL_cast_expression);
    }
  | LPARANTHESIS  type_name  RPARANTHESIS  cast_expression {
      $$ = ast_node_new (YYSYMBOL_cast_expression);
    }
  ;
multiplicative_expression
  : cast_expression {
      $$ = ast_node_new (YYSYMBOL_multiplicative_expression);
    }
  | multiplicative_expression  STAR  cast_expression {
      $$ = ast_node_new (YYSYMBOL_multiplicative_expression);
    }
  | multiplicative_expression  SLASH  cast_expression {
      $$ = ast_node_new (YYSYMBOL_multiplicative_expression);
    }
  | multiplicative_expression  PERCENT  cast_expression {
      $$ = ast_node_new (YYSYMBOL_multiplicative_expression);
    }
  ;
additive_expression
  : multiplicative_expression {
      $$ = ast_node_new (YYSYMBOL_additive_expression);
    }
  | additive_expression  PLUS  multiplicative_expression {
      $$ = ast_node_new (YYSYMBOL_additive_expression);
    }
  | additive_expression  MINUS  multiplicative_expression {
      $$ = ast_node_new (YYSYMBOL_additive_expression);
    }
  ;
shift_expression
  : additive_expression {
      $$ = ast_node_new (YYSYMBOL_shift_expression);
    }
  | shift_expression  LEFT_OP  additive_expression {
      $$ = ast_node_new (YYSYMBOL_shift_expression);
    }
  | shift_expression  RIGHT_OP  additive_expression {
      $$ = ast_node_new (YYSYMBOL_shift_expression);
    }
  ;
relational_expression
  : shift_expression {
      $$ = ast_node_new (YYSYMBOL_relational_expression);
    }
  | relational_expression  L_T  shift_expression {
      $$ = ast_node_new (YYSYMBOL_relational_expression);
    }
  | relational_expression  G_T  shift_expression {
      $$ = ast_node_new (YYSYMBOL_relational_expression);
    }
  | relational_expression  LE_OP  shift_expression {
      $$ = ast_node_new (YYSYMBOL_relational_expression);
    }
  | relational_expression  GE_OP  shift_expression {
      $$ = ast_node_new (YYSYMBOL_relational_expression);
    }
  ;
equality_expression
  : relational_expression {
      $$ = ast_node_new (YYSYMBOL_equality_expression);
    }
  | equality_expression  EQ_OP  relational_expression {
      $$ = ast_node_new (YYSYMBOL_equality_expression);
    }
  | equality_expression  NE_OP  relational_expression {
      $$ = ast_node_new (YYSYMBOL_equality_expression);
    }
  ;
and_expression
  : equality_expression {
      $$ = ast_node_new (YYSYMBOL_and_expression);
    }
  | and_expression  AMPERSAND  equality_expression {
      $$ = ast_node_new (YYSYMBOL_and_expression);
    }
  ;
exclusive_or_expression
  : and_expression {
      $$ = ast_node_new (YYSYMBOL_exclusive_or_expression);
    }
  | exclusive_or_expression  CARET  and_expression {
      $$ = ast_node_new (YYSYMBOL_exclusive_or_expression);
    }
  ;
inclusive_or_expression
  : exclusive_or_expression {
      $$ = ast_node_new (YYSYMBOL_inclusive_or_expression);
    }
  | inclusive_or_expression  PIPE  exclusive_or_expression {
      $$ = ast_node_new (YYSYMBOL_inclusive_or_expression);
    }
  ;
logical_and_expression
  : inclusive_or_expression {
      $$ = ast_node_new (YYSYMBOL_logical_and_expression);
    }
  | logical_and_expression  AND_OP  inclusive_or_expression {
      $$ = ast_node_new (YYSYMBOL_logical_and_expression);
    }
  ;
logical_or_expression
  : logical_and_expression {
      $$ = ast_node_new (YYSYMBOL_logical_or_expression);
    }
  | logical_or_expression  OR_OP  logical_and_expression {
      $$ = ast_node_new (YYSYMBOL_logical_or_expression);
    }
  ;
conditional_expression
  : logical_or_expression {
      $$ = ast_node_new (YYSYMBOL_conditional_expression);
    }
  | logical_or_expression  QUESTION  expression  COLON  conditional_expression {
      $$ = ast_node_new (YYSYMBOL_conditional_expression);
    }
  ;
assignment_expression
  : conditional_expression {
      $$ = ast_node_new (YYSYMBOL_assignment_expression);
    }
  | unary_expression  assignment_operator  assignment_expression {
      $$ = ast_node_new (YYSYMBOL_assignment_expression);
    }
  ;
assignment_operator
  : EQUAL {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | MUL_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | DIV_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | MOD_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | ADD_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | SUB_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | LEFT_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | RIGHT_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | AND_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | XOR_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  | OR_ASSIGN {
      $$ = ast_node_new (YYSYMBOL_assignment_operator);
    }
  ;
expression
  : assignment_expression {
      $$ = ast_node_new (YYSYMBOL_expression);
    }
  | expression  COMMA  assignment_expression {
      $$ = ast_node_new (YYSYMBOL_expression);
    }
  ;
constant_expression
  : conditional_expression {
      $$ = ast_node_new (YYSYMBOL_constant_expression);
    }
  ;
declaration
  : declaration_specifiers  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_declaration);
    }
  | declaration_specifiers  init_declarator_list  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_declaration);
    }
  | static_assert_declaration {
      $$ = ast_node_new (YYSYMBOL_declaration);
    }
  ;
declaration_specifiers
  : storage_class_specifier  declaration_specifiers {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | storage_class_specifier {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | type_specifier  declaration_specifiers {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | type_specifier {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | type_qualifier  declaration_specifiers {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | type_qualifier {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | function_specifier  declaration_specifiers {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | function_specifier {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | alignment_specifier  declaration_specifiers {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  | alignment_specifier {
      $$ = ast_node_new (YYSYMBOL_declaration_specifiers);
    }
  ;
init_declarator_list
  : init_declarator {
      $$ = ast_node_new (YYSYMBOL_init_declarator_list);
    }
  | init_declarator_list  COMMA  init_declarator {
      $$ = ast_node_new (YYSYMBOL_init_declarator_list);
    }
  ;
init_declarator
  : declarator  EQUAL  initializer {
      $$ = ast_node_new (YYSYMBOL_init_declarator);
    }
  | declarator {
      $$ = ast_node_new (YYSYMBOL_init_declarator);
    }
  ;
storage_class_specifier
  : TYPEDEF {
      $$ = ast_node_new (YYSYMBOL_storage_class_specifier);
    }
  | EXTERN {
      $$ = ast_node_new (YYSYMBOL_storage_class_specifier);
    }
  | STATIC {
      $$ = ast_node_new (YYSYMBOL_storage_class_specifier);
    }
  | THREAD_LOCAL {
      $$ = ast_node_new (YYSYMBOL_storage_class_specifier);
    }
  | AUTO {
      $$ = ast_node_new (YYSYMBOL_storage_class_specifier);
    }
  | REGISTER {
      $$ = ast_node_new (YYSYMBOL_storage_class_specifier);
    }
  ;
type_specifier
  : VOID {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | CHAR {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | SHORT {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | INT {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | LONG {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | FLOAT {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | DOUBLE {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | SIGNED {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | UNSIGNED {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | BOOL {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | COMPLEX {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | IMAGINARY {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | atomic_type_specifier {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | struct_or_union_specifier {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | enum_specifier {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  | TYPEDEF_NAME {
      $$ = ast_node_new (YYSYMBOL_type_specifier);
    }
  ;
struct_or_union_specifier
  : struct_or_union  LBRACE  struct_declaration_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_struct_or_union_specifier);
    }
  | struct_or_union  IDENTIFIER  LBRACE  struct_declaration_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_struct_or_union_specifier);
    }
  | struct_or_union  IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_struct_or_union_specifier);
    }
  ;
struct_or_union
  : STRUCT {
      $$ = ast_node_new (YYSYMBOL_struct_or_union);
    }
  | UNION {
      $$ = ast_node_new (YYSYMBOL_struct_or_union);
    }
  ;
struct_declaration_list
  : struct_declaration {
      $$ = ast_node_new (YYSYMBOL_struct_declaration_list);
    }
  | struct_declaration_list  struct_declaration {
      $$ = ast_node_new (YYSYMBOL_struct_declaration_list);
    }
  ;
struct_declaration
  : specifier_qualifier_list  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_struct_declaration);
    }
  | specifier_qualifier_list  struct_declarator_list  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_struct_declaration);
    }
  | static_assert_declaration {
      $$ = ast_node_new (YYSYMBOL_struct_declaration);
    }
  ;
specifier_qualifier_list
  : type_specifier  specifier_qualifier_list {
      $$ = ast_node_new (YYSYMBOL_specifier_qualifier_list);
    }
  | type_specifier {
      $$ = ast_node_new (YYSYMBOL_specifier_qualifier_list);
    }
  | type_qualifier  specifier_qualifier_list {
      $$ = ast_node_new (YYSYMBOL_specifier_qualifier_list);
    }
  | type_qualifier {
      $$ = ast_node_new (YYSYMBOL_specifier_qualifier_list);
    }
  ;
struct_declarator_list
  : struct_declarator {
      $$ = ast_node_new (YYSYMBOL_struct_declarator_list);
    }
  | struct_declarator_list  COMMA  struct_declarator {
      $$ = ast_node_new (YYSYMBOL_struct_declarator_list);
    }
  ;
struct_declarator
  : COLON  constant_expression {
      $$ = ast_node_new (YYSYMBOL_struct_declarator);
    }
  | declarator  COLON  constant_expression {
      $$ = ast_node_new (YYSYMBOL_struct_declarator);
    }
  | declarator {
      $$ = ast_node_new (YYSYMBOL_struct_declarator);
    }
  ;
enum_specifier
  : ENUM  LBRACE  enumerator_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_enum_specifier);
    }
  | ENUM  LBRACE  enumerator_list  COMMA  RBRACE {
      $$ = ast_node_new (YYSYMBOL_enum_specifier);
    }
  | ENUM  IDENTIFIER  LBRACE  enumerator_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_enum_specifier);
    }
  | ENUM  IDENTIFIER  LBRACE  enumerator_list  COMMA  RBRACE {
      $$ = ast_node_new (YYSYMBOL_enum_specifier);
    }
  | ENUM  IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_enum_specifier);
    }
  ;
enumerator_list
  : enumerator {
      $$ = ast_node_new (YYSYMBOL_enumerator_list);
    }
  | enumerator_list  COMMA  enumerator {
      $$ = ast_node_new (YYSYMBOL_enumerator_list);
    }
  ;
enumerator
  : enumeration_constant  EQUAL  constant_expression {
      $$ = ast_node_new (YYSYMBOL_enumerator);
    }
  | enumeration_constant {
      $$ = ast_node_new (YYSYMBOL_enumerator);
    }
  ;
atomic_type_specifier
  : ATOMIC  LPARANTHESIS  type_name  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_atomic_type_specifier);
    }
  ;
type_qualifier
  : CONST {
      $$ = ast_node_new (YYSYMBOL_type_qualifier);
    }
  | RESTRICT {
      $$ = ast_node_new (YYSYMBOL_type_qualifier);
    }
  | VOLATILE {
      $$ = ast_node_new (YYSYMBOL_type_qualifier);
    }
  | ATOMIC {
      $$ = ast_node_new (YYSYMBOL_type_qualifier);
    }
  ;
function_specifier
  : INLINE {
      $$ = ast_node_new (YYSYMBOL_function_specifier);
    }
  | NORETURN {
      $$ = ast_node_new (YYSYMBOL_function_specifier);
    }
  ;
alignment_specifier
  : ALIGNAS  LPARANTHESIS  type_name  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_alignment_specifier);
    }
  | ALIGNAS  LPARANTHESIS  constant_expression  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_alignment_specifier);
    }
  ;
declarator
  : pointer  direct_declarator {
      $$ = ast_node_new (YYSYMBOL_declarator);
    }
  | direct_declarator {
      $$ = ast_node_new (YYSYMBOL_declarator);
    }
  ;
direct_declarator
  : IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | LPARANTHESIS  declarator  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  STAR  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  STATIC  type_qualifier_list  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  STATIC  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  type_qualifier_list  STAR  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  type_qualifier_list  STATIC  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  type_qualifier_list  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  type_qualifier_list  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LBRACKET  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LPARANTHESIS  parameter_type_list  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LPARANTHESIS  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  | direct_declarator  LPARANTHESIS  identifier_list  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_declarator);
    }
  ;
pointer
  : STAR  type_qualifier_list  pointer {
      $$ = ast_node_new (YYSYMBOL_pointer);
    }
  | STAR  type_qualifier_list {
      $$ = ast_node_new (YYSYMBOL_pointer);
    }
  | STAR  pointer {
      $$ = ast_node_new (YYSYMBOL_pointer);
    }
  | STAR {
      $$ = ast_node_new (YYSYMBOL_pointer);
    }
  ;
type_qualifier_list
  : type_qualifier {
      $$ = ast_node_new (YYSYMBOL_type_qualifier_list);
    }
  | type_qualifier_list  type_qualifier {
      $$ = ast_node_new (YYSYMBOL_type_qualifier_list);
    }
  ;
parameter_type_list
  : parameter_list  COMMA  ELLIPSIS {
      $$ = ast_node_new (YYSYMBOL_parameter_type_list);
    }
  | parameter_list {
      $$ = ast_node_new (YYSYMBOL_parameter_type_list);
    }
  ;
parameter_list
  : parameter_declaration {
      $$ = ast_node_new (YYSYMBOL_parameter_list);
    }
  | parameter_list  COMMA  parameter_declaration {
      $$ = ast_node_new (YYSYMBOL_parameter_list);
    }
  ;
parameter_declaration
  : declaration_specifiers  declarator {
      $$ = ast_node_new (YYSYMBOL_parameter_declaration);
    }
  | declaration_specifiers  abstract_declarator {
      $$ = ast_node_new (YYSYMBOL_parameter_declaration);
    }
  | declaration_specifiers {
      $$ = ast_node_new (YYSYMBOL_parameter_declaration);
    }
  ;
identifier_list
  : IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_identifier_list);
    }
  | identifier_list  COMMA  IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_identifier_list);
    }
  ;
type_name
  : specifier_qualifier_list  abstract_declarator {
      $$ = ast_node_new (YYSYMBOL_type_name);
    }
  | specifier_qualifier_list {
      $$ = ast_node_new (YYSYMBOL_type_name);
    }
  ;
abstract_declarator
  : pointer  direct_abstract_declarator {
      $$ = ast_node_new (YYSYMBOL_abstract_declarator);
    }
  | pointer {
      $$ = ast_node_new (YYSYMBOL_abstract_declarator);
    }
  | direct_abstract_declarator {
      $$ = ast_node_new (YYSYMBOL_abstract_declarator);
    }
  ;
direct_abstract_declarator
  : LPARANTHESIS  abstract_declarator  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  STAR  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  STATIC  type_qualifier_list  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  STATIC  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  type_qualifier_list  STATIC  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  type_qualifier_list  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  type_qualifier_list  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LBRACKET  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  STAR  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  STATIC  type_qualifier_list  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  STATIC  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  type_qualifier_list  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  type_qualifier_list  STATIC  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  type_qualifier_list  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LBRACKET  assignment_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LPARANTHESIS  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | LPARANTHESIS  parameter_type_list  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LPARANTHESIS  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  | direct_abstract_declarator  LPARANTHESIS  parameter_type_list  RPARANTHESIS {
      $$ = ast_node_new (YYSYMBOL_direct_abstract_declarator);
    }
  ;
initializer
  : LBRACE  initializer_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_initializer);
    }
  | LBRACE  initializer_list  COMMA  RBRACE {
      $$ = ast_node_new (YYSYMBOL_initializer);
    }
  | assignment_expression {
      $$ = ast_node_new (YYSYMBOL_initializer);
    }
  ;
initializer_list
  : designation  initializer {
      $$ = ast_node_new (YYSYMBOL_initializer_list);
    }
  | initializer {
      $$ = ast_node_new (YYSYMBOL_initializer_list);
    }
  | initializer_list  COMMA  designation  initializer {
      $$ = ast_node_new (YYSYMBOL_initializer_list);
    }
  | initializer_list  COMMA  initializer {
      $$ = ast_node_new (YYSYMBOL_initializer_list);
    }
  ;
designation
  : designator_list  EQUAL {
      $$ = ast_node_new (YYSYMBOL_designation);
    }
  ;
designator_list
  : designator {
      $$ = ast_node_new (YYSYMBOL_designator_list);
    }
  | designator_list  designator {
      $$ = ast_node_new (YYSYMBOL_designator_list);
    }
  ;
designator
  : LBRACKET  constant_expression  RBRACKET {
      $$ = ast_node_new (YYSYMBOL_designator);
    }
  | DOT  IDENTIFIER {
      $$ = ast_node_new (YYSYMBOL_designator);
    }
  ;
static_assert_declaration
  : STATIC_ASSERT  LPARANTHESIS  constant_expression  COMMA  STRING_LITERAL  RPARANTHESIS  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_static_assert_declaration);
    }
  ;
statement
  : labeled_statement {
      $$ = ast_node_new (YYSYMBOL_statement);
    }
  | compound_statement {
      $$ = ast_node_new (YYSYMBOL_statement);
    }
  | expression_statement {
      $$ = ast_node_new (YYSYMBOL_statement);
    }
  | selection_statement {
      $$ = ast_node_new (YYSYMBOL_statement);
    }
  | iteration_statement {
      $$ = ast_node_new (YYSYMBOL_statement);
    }
  | jump_statement {
      $$ = ast_node_new (YYSYMBOL_statement);
    }
  ;
labeled_statement
  : IDENTIFIER  COLON  statement {
      $$ = ast_node_new (YYSYMBOL_labeled_statement);
    }
  | CASE  constant_expression  COLON  statement {
      $$ = ast_node_new (YYSYMBOL_labeled_statement);
    }
  | DEFAULT  COLON  statement {
      $$ = ast_node_new (YYSYMBOL_labeled_statement);
    }
  ;
compound_statement
  : LBRACE  RBRACE {
      $$ = ast_node_new (YYSYMBOL_compound_statement);
    }
  | LBRACE  block_item_list  RBRACE {
      $$ = ast_node_new (YYSYMBOL_compound_statement);
    }
  ;
block_item_list
  : block_item {
      $$ = ast_node_new (YYSYMBOL_block_item_list);
    }
  | block_item_list  block_item {
      $$ = ast_node_new (YYSYMBOL_block_item_list);
    }
  ;
block_item
  : declaration {
      $$ = ast_node_new (YYSYMBOL_block_item);
    }
  | statement {
      $$ = ast_node_new (YYSYMBOL_block_item);
    }
  ;
expression_statement
  : SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_expression_statement);
    }
  | expression  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_expression_statement);
    }
  ;
selection_statement
  : IF  LPARANTHESIS  expression  RPARANTHESIS  statement  ELSE  statement {
      $$ = ast_node_new (YYSYMBOL_selection_statement);
    }
  | IF  LPARANTHESIS  expression  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_selection_statement);
    }
  | SWITCH  LPARANTHESIS  expression  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_selection_statement);
    }
  ;
iteration_statement
  : WHILE  LPARANTHESIS  expression  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_iteration_statement);
    }
  | DO  statement  WHILE  LPARANTHESIS  expression  RPARANTHESIS  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_iteration_statement);
    }
  | FOR  LPARANTHESIS  expression_statement  expression_statement  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_iteration_statement);
    }
  | FOR  LPARANTHESIS  expression_statement  expression_statement  expression  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_iteration_statement);
    }
  | FOR  LPARANTHESIS  declaration  expression_statement  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_iteration_statement);
    }
  | FOR  LPARANTHESIS  declaration  expression_statement  expression  RPARANTHESIS  statement {
      $$ = ast_node_new (YYSYMBOL_iteration_statement);
    }
  ;
jump_statement
  : GOTO  IDENTIFIER  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_jump_statement);
    }
  | CONTINUE  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_jump_statement);
    }
  | BREAK  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_jump_statement);
    }
  | RETURN  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_jump_statement);
    }
  | RETURN  expression  SEMICOLON {
      $$ = ast_node_new (YYSYMBOL_jump_statement);
    }
  ;
root
  : translation_unit { }
  ;
translation_unit
  : external_declaration {
      $$ = ast_node_new (YYSYMBOL_translation_unit);
    }
  | translation_unit  external_declaration {
      $$ = ast_node_new (YYSYMBOL_translation_unit);
    }
  ;
external_declaration
  : function_definition {
      $$ = ast_node_new (YYSYMBOL_external_declaration);
    }
  | declaration {
      $$ = ast_node_new (YYSYMBOL_external_declaration);
    }
  ;
function_definition
  : declaration_specifiers  declarator  declaration_list  compound_statement {
      $$ = ast_node_new (YYSYMBOL_function_definition);
    }
  | declaration_specifiers  declarator  compound_statement {
      $$ = ast_node_new (YYSYMBOL_function_definition);
    }
  ;
declaration_list
  : declaration {
      $$ = ast_node_new (YYSYMBOL_declaration_list);
    }
  | declaration_list  declaration {
      $$ = ast_node_new (YYSYMBOL_declaration_list);
    }
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

    /*
    .. fixme: use status, to see if parser has exited properly
    */
    (void)status;

    /*
    .. AST syntax check, analysis, etc
    */
    return 0;
  }
