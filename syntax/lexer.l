
  /*
  .. This file is created by modifying a UNIX lex source file
  .. (for a std C lexical analyser) written by Jutta Degener. 
  .. Find the file here : https://www.quut.com/c/ANSI-C-grammar-l-1999.html
  .. The matching UNIX yacc file can be found here 
  .. https://www.quut.com/c/ANSI-C-grammar-y-1999.html
  */

  /*
  .. ANSI C grammar, Lex specification
  .. (This lex file is accompanied by a matching yacc file.)
  .. 
  .. In 1985, Jeff Lee published this Lex specification together with a 
  .. Yacc grammar for the April 30, 1985 ANSI C draft. Tom Stockfisch 
  .. reposted both to net.sources in 1987; that original, as mentioned in 
  .. the answer to question 17.25 of the comp.lang.c FAQ, used to be 
  .. available via ftp from ftp.uu.net as usenet/net.sources/ansi.c.grammar.Z
  .. 
  .. The version you see here has been updated based on an 1999 draft of 
  .. the standards document. It allows for restricted pointers, 
  .. variable arrays, "inline", and designated initializers. 
  .. The previous version's lex and yacc files (ANSI C as of ca 1995) are 
  .. still around as archived copies.
  .. 
  .. I want to keep this version as close to the 1999 Standard C grammar 
  .. as possible; please let me know if you discover discrepancies.
  .. (If you feel like it, read the FAQ first.)
  .. 
  .. Jutta Degener, 2017
  .. 
  .. Changes:
  .. Jun 6 2017	 	The exponent of hexadecimal floating point constants 
  .. is not optional (replace {P}? with {P} in two rules).
  .. Kudos to Bryant Wong <bryant@xorshift.org> for spotting the error.
  .. */

  /* we use the following flex options
  .. 1) noyywrap: means flex doesn't expect a yywrap() function
  .. 2) bison-bridge: This instructs flex that the macro yylval may be used. 
  ..    The data type for yylval, YYSTYPE, is defined as { _AstNode * }
  .. 3) nounput: unput() won't be defined by flex.
  */

%option noyywrap bison-bridge nounput

/* some regular expression (regex) that you come across in C */
O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%{

  #include <stdio.h>

  #include <ast.h>
  #include "parser.h"
  #include "symbols.h"

  /* 
  .. The semantic type, YYSTYPE,  (which is 'int' by default),
  .. is defined as '_AstNode *' which represent a pointer of AST node.
  .. NOTE : You have to set the same in parser.y using,
  ..    %define api.value.type {_AstNode *}
  .. The semantic value, yylval_param, of type YYSTYPE, is a local variable 
  .. within yyparse() whose address (i.e &yylval_param) is passed to yylex(). 
  .. So you set *yylval_param for tokens inside yylex(), like below:
  ..    *yylval = AstNode( , , );
  .. AstNode() is the user-defined function that creates an ast node (./ast/ast.h) 
  ..
  .. NOTE: There is a distinction between 
  ..  (a) yylval_param : local variable for semantic value in pure parser
  ..  (b) yylval : global variable for semantic value in impure parser.
  ..
  .. # define YY_DECL
  .. Flex doesn't know the exact function signature unless you define it.
  .. Bison expects to call yylex(...) with those parameters. 
  .. This tells Flex to implement that exact prototype.
  */

  # define YYSTYPE _AstNode *
  # define YY_DECL int yylex (YYSTYPE * yylval_param, _Ast * ast)

  /*  
  .. Warning! : Don't use input(), rather use ast_getchar()
  .. to read a character from the source.
  */
  extern int ast_getchar ( _Ast * ast );

  /* prints grammar violation message. defined in parser.y*/
  extern void yyerror(_Ast *, const char *);  

  /* returns type from symbol table 
  extern int sym_type(const char *);  
  */

  /* with no symbol table, fake it 
  #define sym_type(identifier) IDENTIFIER 
  */

  /* Ignore multi-line comments */
  static void comment(_Ast * );

  /* check_type of the identifier */
  static int check_type(_Ast *, const char *, _AstNode *);

  /* Ignore __attribute__ () */
  static void ignore_attribute(_Ast *);

  /*
  .. These macros will be called inside token rule section,
  .. whenever appropriate tokens are identified by yylex().
  .. fixme: lexer still uses global vars like yytext, yyleng.
  .. Make yylex() reentrant.
  .. fixme: warning: I don't know any tokens that contains 
  .. escape keys other than string literals. 
  */

  /* General tokens */
  #define _TOKEN_(__sym__)                          \
    *yylval = ast_tnode_new (ast, __sym__, yytext); \
    ast->loc.column += yyleng;                      \
    return __sym__;

  /* Token that can possibly contain escape characters*/
  #define _TOKEN_ESC_KEYS_(__sym__)                 \
    *yylval = ast_tnode_new (ast, __sym__, yytext); \
    AstLocRead(yytext);                             \
    return __sym__;

  /* Identifiers */
  #define _TOKEN_IDENTIFIER_()                      \
    *yylval = ast_tnode_new (ast, 0, NULL);         \
    int __sym__ = check_type(ast, yytext, *yylval); \
    ast->loc.column += yyleng;                      \
    return __sym__;

%}


  /* 
  .. In the following tokens rule section that falls b/w "%%" and "%%", 
  .. possible tokens in C are defined. Some tokens are defined as combination of
  .. regex tokens (defined at the beginning of this file).
  .. 
  .. Tokens in C can be,
  ..   - comment
  ..   - Keyword
  ..   - Identifier: func name, var name, etc.
  ..   - Integer Constant. Hex/Octal/Binary.
  ..   - Float Const
  ..   - String 
  ..   - Special Characters
  ..   - Operators. (Also read on ANSI C digraphs, like '<%', '%>', '<:', ':>', etc) 
  ..   - White space
  .. 
  .. The lexical analyser has to take care of some non-C tokens like
  ..   - Compiler specific keywords like attribute, etc. 
  ..     They are just ignored for the moment.
  ..   - gcc specific syntax like:
  ..       # 42 "file.h"
  ..     which specify line number & file name of source code in the in-lined o/p from preprocessor.
  ..     It is used by debugger
  ..   - NOTE: No other non-C grammar is allowed other than the 2 exceptions listed above
  ..
  */

%%

"/*"             { ast->loc.column += 2; comment(ast); }
"//".*           { /* Neglect */ }

"auto"           { _TOKEN_(AUTO); }
"break"          { _TOKEN_(BREAK); }
"case"           { _TOKEN_(CASE); }
"char"           { _TOKEN_(CHAR); }
"const"          { _TOKEN_(CONST); }
"continue"       { _TOKEN_(CONTINUE); }
"default"        { _TOKEN_(DEFAULT); }
"do"             { _TOKEN_(DO); }
"double"         { _TOKEN_(DOUBLE); }
"else"           { _TOKEN_(ELSE); }
"enum"           { _TOKEN_(ENUM); }
"extern"         { _TOKEN_(EXTERN); }
"float"          { _TOKEN_(FLOAT); }
"for"            { _TOKEN_(FOR); }
"goto"           { _TOKEN_(GOTO); }
"if"             { _TOKEN_(IF); }
"inline"         { _TOKEN_(INLINE); }
"int"            { _TOKEN_(INT); }
"long"           { _TOKEN_(LONG); }
"register"       { _TOKEN_(REGISTER); }
"restrict"       { _TOKEN_(RESTRICT); }
"return"         { _TOKEN_(RETURN); }
"short"          { _TOKEN_(SHORT); }
"signed"         { _TOKEN_(SIGNED); }
"sizeof"         { _TOKEN_(SIZEOF); }
"static"         { _TOKEN_(STATIC); }
"struct"         { _TOKEN_(STRUCT); }
"switch"         { _TOKEN_(SWITCH); }
"typedef"        { _TOKEN_(TYPEDEF); }
"union"          { _TOKEN_(UNION); }
"unsigned"       { _TOKEN_(UNSIGNED); }
"void"           { _TOKEN_(VOID); }
"volatile"       { _TOKEN_(VOLATILE); }
"while"          { _TOKEN_(WHILE); }
"_Alignas"                              { _TOKEN_(ALIGNAS); }
"_Alignof"                              { _TOKEN_(ALIGNOF); }
"_Atomic"                               { _TOKEN_(ATOMIC); }
"_Bool"                                 { _TOKEN_(BOOL); }
"_Complex"                              { _TOKEN_(COMPLEX); }
"_Generic"                              { _TOKEN_(GENERIC); }
"_Imaginary"                            { _TOKEN_(IMAGINARY); }
"_Noreturn"                             { _TOKEN_(NORETURN); }
"_Static_assert"                        { _TOKEN_(STATIC_ASSERT); }
"_Thread_local"                         { _TOKEN_(THREAD_LOCAL); }
"__func__"                              { _TOKEN_(FUNC_NAME); }

{L}{A}*                             { _TOKEN_IDENTIFIER_(); }

{HP}{H}+{IS}?                       { _TOKEN_(I_CONSTANT); }
{NZ}{D}*{IS}?                       { _TOKEN_(I_CONSTANT); }
"0"{O}*{IS}?                        { _TOKEN_(I_CONSTANT); }
{CP}?"'"([^'\\\n]|{ES})+"'"         { _TOKEN_ESC_KEYS_(I_CONSTANT); }

{D}+{E}{FS}?                        { _TOKEN_(F_CONSTANT); }
{D}*"."{D}+{E}?{FS}?                { _TOKEN_(F_CONSTANT); }
{D}+"."{E}?{FS}?                    { _TOKEN_(F_CONSTANT); }
{HP}{H}+{P}{FS}?                    { _TOKEN_(F_CONSTANT); }
{HP}{H}*"."{H}+{P}{FS}?             { _TOKEN_(F_CONSTANT); }
{HP}{H}+"."{P}{FS}?                 { _TOKEN_(F_CONSTANT); }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+   { _TOKEN_ESC_KEYS_(STRING_LITERAL); }

"..."          { _TOKEN_(ELLIPSIS); }
">>="          { _TOKEN_(RIGHT_ASSIGN); }
"<<="          { _TOKEN_(LEFT_ASSIGN); }
"+="           { _TOKEN_(ADD_ASSIGN); }
"-="           { _TOKEN_(SUB_ASSIGN); }
"*="           { _TOKEN_(MUL_ASSIGN); }
"/="           { _TOKEN_(DIV_ASSIGN); }
"%="           { _TOKEN_(MOD_ASSIGN); }
"&="           { _TOKEN_(AND_ASSIGN); }
"^="           { _TOKEN_(XOR_ASSIGN); }
"|="           { _TOKEN_(OR_ASSIGN); }
">>"           { _TOKEN_(RIGHT_OP); }
"<<"           { _TOKEN_(LEFT_OP); }
"++"           { _TOKEN_(INC_OP); }
"--"           { _TOKEN_(DEC_OP); }
"->"           { _TOKEN_(PTR_OP); }
"&&"           { _TOKEN_(AND_OP); }
"||"           { _TOKEN_(OR_OP); }
"<="           { _TOKEN_(LE_OP); }
">="           { _TOKEN_(GE_OP); }
"=="           { _TOKEN_(EQ_OP); }
"!="           { _TOKEN_(NE_OP); }

";"            { _TOKEN_(SEMICOLON); }
("{"|"<%")     { _TOKEN_(LBRACE); }
("}"|"%>")     { _TOKEN_(RBRACE); }
","            { _TOKEN_(COMMA); }
":"            { _TOKEN_(COLON); }
"="            { _TOKEN_(EQUAL); }
"("            { _TOKEN_(LPARENTHESIS); }
")"            { _TOKEN_(RPARENTHESIS); }
("["|"<:")     { _TOKEN_(LBRACKET); }
("]"|":>")     { _TOKEN_(RBRACKET); }
"."            { _TOKEN_(DOT); }
"&"            { _TOKEN_(AMPERSAND); }
"!"            { _TOKEN_(NOT); }
"~"            { _TOKEN_(TILDE); }
"-"            { _TOKEN_(MINUS); }
"+"            { _TOKEN_(PLUS); }
"*"            { _TOKEN_(STAR); }
"/"            { _TOKEN_(SLASH); }
"%"            { _TOKEN_(PERCENT); }
"<"            { _TOKEN_(L_T); }
">"            { _TOKEN_(G_T); }
"^"            { _TOKEN_(CARET); }
"|"            { _TOKEN_(PIPE); }
"?"            { _TOKEN_(QUESTION); }

([ \v\f\n\t])+  { AstLocRead(yytext); }

        
  /** 
  .......................................
  ........ Some non-C grammars ..........
  .......................................
  **/

^[ \t]*#[ \t]+[0-9]+[ \t]+\"([^"\\\n]|{ES})*\".*  { 
  ast_reset_source(ast, yytext);
}

"__attribute__"{WS}*\(                            {
  /* 
  .. Fixme :  attributes should be rather,
  .. forwarded to the compiler 
  */
  ast->loc.column += yyleng;
  ignore_attribute (ast);
}

  /** 
  .......................................
  ....... End of non-C grammars ........
  .......................................
  **/


.                                                  {
  ++(ast->loc.column); 
  /*
  .. Discard bad characters. 
  .. It should be placed at the end of the tokens list. It discards/ignore
  .. any tokens that doesn't match any of the previous tokens
  */ 
}

%%
  
  /* 
  .. yywrap() nor required, since we have declared "%option noyywarp".
  .. int yywrap(void) { return 1; }
  */
  
  /* 
  .. This function ignores attributes: "__attribute__ ()"
  .. which are meant for gcc/clang compilers.
  .. fixme : They should rather be forwarded as it is to the compiler.
  */

  static void ignore_attribute(_Ast * ast)
  {
    int line   = ast->loc.line;
    int column = ast->loc.column;
    char c = 1, nscope = 1;
    while (nscope && c) {
      c = ast_getchar(ast);
      if (c == ')') 
        nscope--;
      else if (c == '(')
        nscope++;
    }
  
    if(nscope) { 
      ast->loc.line   = line;
      ast->loc.column = column;
      yyerror (ast, "__attribute__ not closed");
    }
  }

  
  /* 
  .. This function 'consumes' (skips) comments 
  */
  static void comment(_Ast * ast) {
    int c;
    int line   = ast->loc.line;
    int column = ast->loc.column;
    while ((c = ast_getchar(ast)) != 0)
      if (c == '*')
        {
          while ((c = ast_getchar(ast)) == '*')
            ;
          if (c == '/')
            return;
          if (c == 0)
            break;
        }
    /* Point the error to the location where comments started */
    ast->loc.line   = line;
    ast->loc.column = column;
    yyerror(ast, "unterminated comment");
  }
  
  /* which type of identifier ? */
  static 
  int check_type(_Ast * ast, const char * id, _AstNode * node) {
    _Scope * scope = ast->scope;
    _HashNode * h = NULL;
    do {
      if( (h = hash_lookup (scope->symbols, id)) )  {
        ((_AstTNode *) node)->token = h->key;
        return ( node->symbol = h->symbol ); 
      }
    } while ( (scope = scope->parent) );
    ((_AstTNode *)node)->token = ast_strdup (id) ;
    return ( node->symbol = IDENTIFIER ); 
  }

  int ast_getchar(_Ast * ast) {
    int c = input();
    AstLocInput(c);
    return c;
  }
