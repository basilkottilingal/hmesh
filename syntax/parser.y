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

primary_expression
  : IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_primary_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | constant {
      $$ = ast_node_new (ast, YYSYMBOL_primary_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | string {
      $$ = ast_node_new (ast, YYSYMBOL_primary_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | LPARENTHESIS expression RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_primary_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | generic_selection {
      $$ = ast_node_new (ast, YYSYMBOL_primary_expression, 1);
      ast_node_children($$, 1, $1);
    }
  ;

constant
  : I_CONSTANT {
      $$ = ast_node_new (ast, YYSYMBOL_constant, 1);
      ast_node_children($$, 1, $1);
    }
  | F_CONSTANT {
      $$ = ast_node_new (ast, YYSYMBOL_constant, 1);
      ast_node_children($$, 1, $1);
    }
  | ENUMERATION_CONSTANT {
      $$ = ast_node_new (ast, YYSYMBOL_constant, 1);
      ast_node_children($$, 1, $1);
    }
  ;

enumeration_constant
  : IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_enumeration_constant, 1);
      ast_node_children($$, 1, $1);
    }
  ;

string
  : STRING_LITERAL {
      $$ = ast_node_new (ast, YYSYMBOL_string, 1);
      ast_node_children($$, 1, $1);
    }
  | FUNC_NAME {
      $$ = ast_node_new (ast, YYSYMBOL_string, 1);
      ast_node_children($$, 1, $1);
    }
  ;

generic_selection
  : GENERIC LPARENTHESIS assignment_expression COMMA generic_assoc_list RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_generic_selection, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  ;

generic_assoc_list
  : generic_association {
      $$ = ast_node_new (ast, YYSYMBOL_generic_assoc_list, 1);
      ast_node_children($$, 1, $1);
    }
  | generic_assoc_list COMMA generic_association {
      $$ = ast_node_new (ast, YYSYMBOL_generic_assoc_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

generic_association
  : type_name COLON assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_generic_association, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | DEFAULT COLON assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_generic_association, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

postfix_expression
  : primary_expression {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | postfix_expression LBRACKET expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | postfix_expression LPARENTHESIS RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | postfix_expression LPARENTHESIS argument_expression_list RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | postfix_expression DOT IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | postfix_expression PTR_OP IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | postfix_expression INC_OP {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | postfix_expression DEC_OP {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | LPARENTHESIS type_name RPARENTHESIS LBRACE initializer_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | LPARENTHESIS type_name RPARENTHESIS LBRACE initializer_list COMMA RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_postfix_expression, 7);
      ast_node_children($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
  ;

argument_expression_list
  : assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_argument_expression_list, 1);
      ast_node_children($$, 1, $1);
    }
  | argument_expression_list COMMA assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_argument_expression_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

unary_expression
  : postfix_expression {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | INC_OP unary_expression {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | DEC_OP unary_expression {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | unary_operator cast_expression {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | SIZEOF unary_expression {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | SIZEOF LPARENTHESIS type_name RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | ALIGNOF LPARENTHESIS type_name RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_unary_expression, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  ;

unary_operator
  : AMPERSAND {
      $$ = ast_node_new (ast, YYSYMBOL_unary_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | STAR {
      $$ = ast_node_new (ast, YYSYMBOL_unary_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | PLUS {
      $$ = ast_node_new (ast, YYSYMBOL_unary_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | MINUS {
      $$ = ast_node_new (ast, YYSYMBOL_unary_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | TILDE {
      $$ = ast_node_new (ast, YYSYMBOL_unary_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | NOT {
      $$ = ast_node_new (ast, YYSYMBOL_unary_operator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

cast_expression
  : unary_expression {
      $$ = ast_node_new (ast, YYSYMBOL_cast_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | LPARENTHESIS type_name RPARENTHESIS cast_expression {
      $$ = ast_node_new (ast, YYSYMBOL_cast_expression, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  ;

multiplicative_expression
  : cast_expression {
      $$ = ast_node_new (ast, YYSYMBOL_multiplicative_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | multiplicative_expression STAR cast_expression {
      $$ = ast_node_new (ast, YYSYMBOL_multiplicative_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | multiplicative_expression SLASH cast_expression {
      $$ = ast_node_new (ast, YYSYMBOL_multiplicative_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | multiplicative_expression PERCENT cast_expression {
      $$ = ast_node_new (ast, YYSYMBOL_multiplicative_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

additive_expression
  : multiplicative_expression {
      $$ = ast_node_new (ast, YYSYMBOL_additive_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | additive_expression PLUS multiplicative_expression {
      $$ = ast_node_new (ast, YYSYMBOL_additive_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | additive_expression MINUS multiplicative_expression {
      $$ = ast_node_new (ast, YYSYMBOL_additive_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

shift_expression
  : additive_expression {
      $$ = ast_node_new (ast, YYSYMBOL_shift_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | shift_expression LEFT_OP additive_expression {
      $$ = ast_node_new (ast, YYSYMBOL_shift_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | shift_expression RIGHT_OP additive_expression {
      $$ = ast_node_new (ast, YYSYMBOL_shift_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

relational_expression
  : shift_expression {
      $$ = ast_node_new (ast, YYSYMBOL_relational_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | relational_expression L_T shift_expression {
      $$ = ast_node_new (ast, YYSYMBOL_relational_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | relational_expression G_T shift_expression {
      $$ = ast_node_new (ast, YYSYMBOL_relational_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | relational_expression LE_OP shift_expression {
      $$ = ast_node_new (ast, YYSYMBOL_relational_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | relational_expression GE_OP shift_expression {
      $$ = ast_node_new (ast, YYSYMBOL_relational_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

equality_expression
  : relational_expression {
      $$ = ast_node_new (ast, YYSYMBOL_equality_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | equality_expression EQ_OP relational_expression {
      $$ = ast_node_new (ast, YYSYMBOL_equality_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | equality_expression NE_OP relational_expression {
      $$ = ast_node_new (ast, YYSYMBOL_equality_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

and_expression
  : equality_expression {
      $$ = ast_node_new (ast, YYSYMBOL_and_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | and_expression AMPERSAND equality_expression {
      $$ = ast_node_new (ast, YYSYMBOL_and_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

exclusive_or_expression
  : and_expression {
      $$ = ast_node_new (ast, YYSYMBOL_exclusive_or_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | exclusive_or_expression CARET and_expression {
      $$ = ast_node_new (ast, YYSYMBOL_exclusive_or_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

inclusive_or_expression
  : exclusive_or_expression {
      $$ = ast_node_new (ast, YYSYMBOL_inclusive_or_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | inclusive_or_expression PIPE exclusive_or_expression {
      $$ = ast_node_new (ast, YYSYMBOL_inclusive_or_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

logical_and_expression
  : inclusive_or_expression {
      $$ = ast_node_new (ast, YYSYMBOL_logical_and_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | logical_and_expression AND_OP inclusive_or_expression {
      $$ = ast_node_new (ast, YYSYMBOL_logical_and_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

logical_or_expression
  : logical_and_expression {
      $$ = ast_node_new (ast, YYSYMBOL_logical_or_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | logical_or_expression OR_OP logical_and_expression {
      $$ = ast_node_new (ast, YYSYMBOL_logical_or_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

conditional_expression
  : logical_or_expression {
      $$ = ast_node_new (ast, YYSYMBOL_conditional_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | logical_or_expression QUESTION expression COLON conditional_expression {
      $$ = ast_node_new (ast, YYSYMBOL_conditional_expression, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  ;

assignment_expression
  : conditional_expression {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | unary_expression assignment_operator assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

assignment_operator
  : EQUAL {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | MUL_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | DIV_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | MOD_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | ADD_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | SUB_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | LEFT_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | RIGHT_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | AND_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | XOR_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  | OR_ASSIGN {
      $$ = ast_node_new (ast, YYSYMBOL_assignment_operator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

expression
  : assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_expression, 1);
      ast_node_children($$, 1, $1);
    }
  | expression COMMA assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_expression, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

constant_expression
  : conditional_expression {
      $$ = ast_node_new (ast, YYSYMBOL_constant_expression, 1);
      ast_node_children($$, 1, $1);
    }
  ;

declaration
  : declaration_specifiers SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_declaration, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | declaration_specifiers init_declarator_list SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_declaration, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | static_assert_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_declaration, 1);
      ast_node_children($$, 1, $1);
    }
  ;

declaration_specifiers
  : storage_class_specifier declaration_specifiers {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | storage_class_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 1);
      ast_node_children($$, 1, $1);
    }
  | type_specifier declaration_specifiers {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | type_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 1);
      ast_node_children($$, 1, $1);
    }
  | type_qualifier declaration_specifiers {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | type_qualifier {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 1);
      ast_node_children($$, 1, $1);
    }
  | function_specifier declaration_specifiers {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | function_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 1);
      ast_node_children($$, 1, $1);
    }
  | alignment_specifier declaration_specifiers {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | alignment_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_specifiers, 1);
      ast_node_children($$, 1, $1);
    }
  ;

init_declarator_list
  : init_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_init_declarator_list, 1);
      ast_node_children($$, 1, $1);
    }
  | init_declarator_list COMMA init_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_init_declarator_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

init_declarator
  : declarator EQUAL initializer {
      $$ = ast_node_new (ast, YYSYMBOL_init_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | declarator {
      $$ = ast_node_new (ast, YYSYMBOL_init_declarator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

storage_class_specifier
  : TYPEDEF {
      $$ = ast_node_new (ast, YYSYMBOL_storage_class_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | EXTERN {
      $$ = ast_node_new (ast, YYSYMBOL_storage_class_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | STATIC {
      $$ = ast_node_new (ast, YYSYMBOL_storage_class_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | THREAD_LOCAL {
      $$ = ast_node_new (ast, YYSYMBOL_storage_class_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | AUTO {
      $$ = ast_node_new (ast, YYSYMBOL_storage_class_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | REGISTER {
      $$ = ast_node_new (ast, YYSYMBOL_storage_class_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  ;

type_specifier
  : VOID {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | CHAR {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | SHORT {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | INT {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | LONG {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | FLOAT {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | DOUBLE {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | SIGNED {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | UNSIGNED {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | BOOL {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | COMPLEX {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | IMAGINARY {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | atomic_type_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | struct_or_union_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | enum_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | TYPEDEF_NAME {
      $$ = ast_node_new (ast, YYSYMBOL_type_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  ;

struct_or_union_specifier
  : struct_or_union LBRACE struct_declaration_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_struct_or_union_specifier, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | struct_or_union IDENTIFIER LBRACE struct_declaration_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_struct_or_union_specifier, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | struct_or_union IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_struct_or_union_specifier, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

struct_or_union
  : STRUCT {
      $$ = ast_node_new (ast, YYSYMBOL_struct_or_union, 1);
      ast_node_children($$, 1, $1);
    }
  | UNION {
      $$ = ast_node_new (ast, YYSYMBOL_struct_or_union, 1);
      ast_node_children($$, 1, $1);
    }
  ;

struct_declaration_list
  : struct_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declaration_list, 1);
      ast_node_children($$, 1, $1);
    }
  | struct_declaration_list struct_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declaration_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

struct_declaration
  : specifier_qualifier_list SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declaration, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | specifier_qualifier_list struct_declarator_list SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declaration, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | static_assert_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declaration, 1);
      ast_node_children($$, 1, $1);
    }
  ;

specifier_qualifier_list
  : type_specifier specifier_qualifier_list {
      $$ = ast_node_new (ast, YYSYMBOL_specifier_qualifier_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | type_specifier {
      $$ = ast_node_new (ast, YYSYMBOL_specifier_qualifier_list, 1);
      ast_node_children($$, 1, $1);
    }
  | type_qualifier specifier_qualifier_list {
      $$ = ast_node_new (ast, YYSYMBOL_specifier_qualifier_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | type_qualifier {
      $$ = ast_node_new (ast, YYSYMBOL_specifier_qualifier_list, 1);
      ast_node_children($$, 1, $1);
    }
  ;

struct_declarator_list
  : struct_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declarator_list, 1);
      ast_node_children($$, 1, $1);
    }
  | struct_declarator_list COMMA struct_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declarator_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

struct_declarator
  : COLON constant_expression {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declarator, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | declarator COLON constant_expression {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | declarator {
      $$ = ast_node_new (ast, YYSYMBOL_struct_declarator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

enum_specifier
  : ENUM LBRACE enumerator_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_enum_specifier, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | ENUM LBRACE enumerator_list COMMA RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_enum_specifier, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | ENUM IDENTIFIER LBRACE enumerator_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_enum_specifier, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | ENUM IDENTIFIER LBRACE enumerator_list COMMA RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_enum_specifier, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | ENUM IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_enum_specifier, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

enumerator_list
  : enumerator {
      $$ = ast_node_new (ast, YYSYMBOL_enumerator_list, 1);
      ast_node_children($$, 1, $1);
    }
  | enumerator_list COMMA enumerator {
      $$ = ast_node_new (ast, YYSYMBOL_enumerator_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

enumerator
  : enumeration_constant EQUAL constant_expression {
      $$ = ast_node_new (ast, YYSYMBOL_enumerator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | enumeration_constant {
      $$ = ast_node_new (ast, YYSYMBOL_enumerator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

atomic_type_specifier
  : ATOMIC LPARENTHESIS type_name RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_atomic_type_specifier, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  ;

type_qualifier
  : CONST {
      $$ = ast_node_new (ast, YYSYMBOL_type_qualifier, 1);
      ast_node_children($$, 1, $1);
    }
  | RESTRICT {
      $$ = ast_node_new (ast, YYSYMBOL_type_qualifier, 1);
      ast_node_children($$, 1, $1);
    }
  | VOLATILE {
      $$ = ast_node_new (ast, YYSYMBOL_type_qualifier, 1);
      ast_node_children($$, 1, $1);
    }
  | ATOMIC {
      $$ = ast_node_new (ast, YYSYMBOL_type_qualifier, 1);
      ast_node_children($$, 1, $1);
    }
  ;

function_specifier
  : INLINE {
      $$ = ast_node_new (ast, YYSYMBOL_function_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  | NORETURN {
      $$ = ast_node_new (ast, YYSYMBOL_function_specifier, 1);
      ast_node_children($$, 1, $1);
    }
  ;

alignment_specifier
  : ALIGNAS LPARENTHESIS type_name RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_alignment_specifier, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | ALIGNAS LPARENTHESIS constant_expression RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_alignment_specifier, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  ;

declarator
  : pointer direct_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_declarator, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | direct_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_declarator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

direct_declarator
  : IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 1);
      ast_node_children($$, 1, $1);
    }
  | LPARENTHESIS declarator RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_declarator LBRACKET RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_declarator LBRACKET STAR RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | direct_declarator LBRACKET STATIC type_qualifier_list assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | direct_declarator LBRACKET STATIC assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | direct_declarator LBRACKET type_qualifier_list STAR RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | direct_declarator LBRACKET type_qualifier_list STATIC assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | direct_declarator LBRACKET type_qualifier_list assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | direct_declarator LBRACKET type_qualifier_list RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | direct_declarator LBRACKET assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | direct_declarator LPARENTHESIS parameter_type_list RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | direct_declarator LPARENTHESIS RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_declarator LPARENTHESIS identifier_list RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  ;

pointer
  : STAR type_qualifier_list pointer {
      $$ = ast_node_new (ast, YYSYMBOL_pointer, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | STAR type_qualifier_list {
      $$ = ast_node_new (ast, YYSYMBOL_pointer, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | STAR pointer {
      $$ = ast_node_new (ast, YYSYMBOL_pointer, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | STAR {
      $$ = ast_node_new (ast, YYSYMBOL_pointer, 1);
      ast_node_children($$, 1, $1);
    }
  ;

type_qualifier_list
  : type_qualifier {
      $$ = ast_node_new (ast, YYSYMBOL_type_qualifier_list, 1);
      ast_node_children($$, 1, $1);
    }
  | type_qualifier_list type_qualifier {
      $$ = ast_node_new (ast, YYSYMBOL_type_qualifier_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

parameter_type_list
  : parameter_list COMMA ELLIPSIS {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_type_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | parameter_list {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_type_list, 1);
      ast_node_children($$, 1, $1);
    }
  ;

parameter_list
  : parameter_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_list, 1);
      ast_node_children($$, 1, $1);
    }
  | parameter_list COMMA parameter_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

parameter_declaration
  : declaration_specifiers declarator {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_declaration, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | declaration_specifiers abstract_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_declaration, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | declaration_specifiers {
      $$ = ast_node_new (ast, YYSYMBOL_parameter_declaration, 1);
      ast_node_children($$, 1, $1);
    }
  ;

identifier_list
  : IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_identifier_list, 1);
      ast_node_children($$, 1, $1);
    }
  | identifier_list COMMA IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_identifier_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

type_name
  : specifier_qualifier_list abstract_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_type_name, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | specifier_qualifier_list {
      $$ = ast_node_new (ast, YYSYMBOL_type_name, 1);
      ast_node_children($$, 1, $1);
    }
  ;

abstract_declarator
  : pointer direct_abstract_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_abstract_declarator, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | pointer {
      $$ = ast_node_new (ast, YYSYMBOL_abstract_declarator, 1);
      ast_node_children($$, 1, $1);
    }
  | direct_abstract_declarator {
      $$ = ast_node_new (ast, YYSYMBOL_abstract_declarator, 1);
      ast_node_children($$, 1, $1);
    }
  ;

direct_abstract_declarator
  : LPARENTHESIS abstract_declarator RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | LBRACKET RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | LBRACKET STAR RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | LBRACKET STATIC type_qualifier_list assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | LBRACKET STATIC assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | LBRACKET type_qualifier_list STATIC assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | LBRACKET type_qualifier_list assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | LBRACKET type_qualifier_list RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | LBRACKET assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_abstract_declarator LBRACKET RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_abstract_declarator LBRACKET STAR RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | direct_abstract_declarator LBRACKET STATIC type_qualifier_list assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | direct_abstract_declarator LBRACKET STATIC assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | direct_abstract_declarator LBRACKET type_qualifier_list assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | direct_abstract_declarator LBRACKET type_qualifier_list STATIC assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | direct_abstract_declarator LBRACKET type_qualifier_list RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | direct_abstract_declarator LBRACKET assignment_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | LPARENTHESIS RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | LPARENTHESIS parameter_type_list RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_abstract_declarator LPARENTHESIS RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | direct_abstract_declarator LPARENTHESIS parameter_type_list RPARENTHESIS {
      $$ = ast_node_new (ast, YYSYMBOL_direct_abstract_declarator, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  ;

initializer
  : LBRACE initializer_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_initializer, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | LBRACE initializer_list COMMA RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_initializer, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | assignment_expression {
      $$ = ast_node_new (ast, YYSYMBOL_initializer, 1);
      ast_node_children($$, 1, $1);
    }
  ;

initializer_list
  : designation initializer {
      $$ = ast_node_new (ast, YYSYMBOL_initializer_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | initializer {
      $$ = ast_node_new (ast, YYSYMBOL_initializer_list, 1);
      ast_node_children($$, 1, $1);
    }
  | initializer_list COMMA designation initializer {
      $$ = ast_node_new (ast, YYSYMBOL_initializer_list, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | initializer_list COMMA initializer {
      $$ = ast_node_new (ast, YYSYMBOL_initializer_list, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

designation
  : designator_list EQUAL {
      $$ = ast_node_new (ast, YYSYMBOL_designation, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

designator_list
  : designator {
      $$ = ast_node_new (ast, YYSYMBOL_designator_list, 1);
      ast_node_children($$, 1, $1);
    }
  | designator_list designator {
      $$ = ast_node_new (ast, YYSYMBOL_designator_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

designator
  : LBRACKET constant_expression RBRACKET {
      $$ = ast_node_new (ast, YYSYMBOL_designator, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | DOT IDENTIFIER {
      $$ = ast_node_new (ast, YYSYMBOL_designator, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

static_assert_declaration
  : STATIC_ASSERT LPARENTHESIS constant_expression COMMA STRING_LITERAL RPARENTHESIS SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_static_assert_declaration, 7);
      ast_node_children($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
  ;

statement
  : labeled_statement {
      $$ = ast_node_new (ast, YYSYMBOL_statement, 1);
      ast_node_children($$, 1, $1);
    }
  | compound_statement {
      $$ = ast_node_new (ast, YYSYMBOL_statement, 1);
      ast_node_children($$, 1, $1);
    }
  | expression_statement {
      $$ = ast_node_new (ast, YYSYMBOL_statement, 1);
      ast_node_children($$, 1, $1);
    }
  | selection_statement {
      $$ = ast_node_new (ast, YYSYMBOL_statement, 1);
      ast_node_children($$, 1, $1);
    }
  | iteration_statement {
      $$ = ast_node_new (ast, YYSYMBOL_statement, 1);
      ast_node_children($$, 1, $1);
    }
  | jump_statement {
      $$ = ast_node_new (ast, YYSYMBOL_statement, 1);
      ast_node_children($$, 1, $1);
    }
  ;

labeled_statement
  : IDENTIFIER COLON statement {
      $$ = ast_node_new (ast, YYSYMBOL_labeled_statement, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | CASE constant_expression COLON statement {
      $$ = ast_node_new (ast, YYSYMBOL_labeled_statement, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | DEFAULT COLON statement {
      $$ = ast_node_new (ast, YYSYMBOL_labeled_statement, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

compound_statement
  : LBRACE RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_compound_statement, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | LBRACE block_item_list RBRACE {
      $$ = ast_node_new (ast, YYSYMBOL_compound_statement, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

block_item_list
  : block_item {
      $$ = ast_node_new (ast, YYSYMBOL_block_item_list, 1);
      ast_node_children($$, 1, $1);
    }
  | block_item_list block_item {
      $$ = ast_node_new (ast, YYSYMBOL_block_item_list, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

block_item
  : declaration {
      $$ = ast_node_new (ast, YYSYMBOL_block_item, 1);
      ast_node_children($$, 1, $1);
    }
  | statement {
      $$ = ast_node_new (ast, YYSYMBOL_block_item, 1);
      ast_node_children($$, 1, $1);
    }
  ;

expression_statement
  : SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_expression_statement, 1);
      ast_node_children($$, 1, $1);
    }
  | expression SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_expression_statement, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

selection_statement
  : IF LPARENTHESIS expression RPARENTHESIS statement ELSE statement {
      $$ = ast_node_new (ast, YYSYMBOL_selection_statement, 7);
      ast_node_children($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
  | IF LPARENTHESIS expression RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_selection_statement, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | SWITCH LPARENTHESIS expression RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_selection_statement, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  ;

iteration_statement
  : WHILE LPARENTHESIS expression RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_iteration_statement, 5);
      ast_node_children($$, 5, $1, $2, $3, $4, $5);
    }
  | DO statement WHILE LPARENTHESIS expression RPARENTHESIS SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_iteration_statement, 7);
      ast_node_children($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
  | FOR LPARENTHESIS expression_statement expression_statement RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_iteration_statement, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | FOR LPARENTHESIS expression_statement expression_statement expression RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_iteration_statement, 7);
      ast_node_children($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
  | FOR LPARENTHESIS declaration expression_statement RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_iteration_statement, 6);
      ast_node_children($$, 6, $1, $2, $3, $4, $5, $6);
    }
  | FOR LPARENTHESIS declaration expression_statement expression RPARENTHESIS statement {
      $$ = ast_node_new (ast, YYSYMBOL_iteration_statement, 7);
      ast_node_children($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
  ;

jump_statement
  : GOTO IDENTIFIER SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_jump_statement, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  | CONTINUE SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_jump_statement, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | BREAK SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_jump_statement, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | RETURN SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_jump_statement, 2);
      ast_node_children($$, 2, $1, $2);
    }
  | RETURN expression SEMICOLON {
      $$ = ast_node_new (ast, YYSYMBOL_jump_statement, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

root
  : translation_unit { 
      $$ = &ast->root;
      $$->symbol = YYSYMBOL_root;
      $$->child[0] = $1;
      $1->parent = $$;
    }
  ;

translation_unit
  : external_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_translation_unit, 1);
      ast_node_children($$, 1, $1);
    }
  | translation_unit external_declaration {
      $$ = ast_node_new (ast, YYSYMBOL_translation_unit, 2);
      ast_node_children($$, 2, $1, $2);
    }
  ;

external_declaration
  : function_definition {
      $$ = ast_node_new (ast, YYSYMBOL_external_declaration, 1);
      ast_node_children($$, 1, $1);
    }
  | declaration {
      $$ = ast_node_new (ast, YYSYMBOL_external_declaration, 1);
      ast_node_children($$, 1, $1);
    }
  ;

function_definition
  : declaration_specifiers declarator declaration_list compound_statement {
      $$ = ast_node_new (ast, YYSYMBOL_function_definition, 4);
      ast_node_children($$, 4, $1, $2, $3, $4);
    }
  | declaration_specifiers declarator compound_statement {
      $$ = ast_node_new (ast, YYSYMBOL_function_definition, 3);
      ast_node_children($$, 3, $1, $2, $3);
    }
  ;

declaration_list
  : declaration {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_list, 1);
      ast_node_children($$, 1, $1);
    }
  | declaration_list declaration {
      $$ = ast_node_new (ast, YYSYMBOL_declaration_list, 2);
      ast_node_children($$, 2, $1, $2);
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
    ast_print (ast);

    /*
    .. AST syntax check, analysis, etc
    */
    return 0;
  }
