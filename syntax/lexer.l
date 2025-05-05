
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

  /*
  .. noyywrap: means flex doesn't expect a yywrap() function
  .. yylineno: yylex keeps track of lineno. 
  ..    Updates, i.e yylineno++, whenever input() reads a '\n'
  .. bison-bridge:
  .. nounput: unput() not available
  */

%option noyywrap
%option yylineno
%option bison-bridge
%option nounput


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
  
  /* 
  .. There is no implcit column number tracker 
  .. like yylineno in flex
  */

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

  /* prints grammar violation message. defined in parser.y*/
  extern void yyerror(_Ast *, const char *);  

  /* returns type from symbol table */
  extern int sym_type(const char *);  

  /* with no symbol table, fake it */
  #define sym_type(identifier) IDENTIFIER 

  /* Ignore multi-line comments */
  static void comment(_Ast * );

  /* check_type of the identifier */
  static int check_type(void);

  /* Ignore __attribute__ () */
  static void ignore_attribute(_Ast *);

  /* 
  .. set location of parser whenever tokens like 
  ..   # 5 "header.h"
  .. are encountered
  */
  static void reset_file_line(_Ast *, const char *);

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
  ..   - Operators
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

"/*"             { comment(ast); }
"//".*           { /* consume //-comment */ }

"auto"           { return(AUTO); }
"break"          { return(BREAK); }
"case"           { return(CASE); }
"char"           { return(CHAR); }
"const"          { return(CONST); }
"continue"       { return(CONTINUE); }
"default"        { return(DEFAULT); }
"do"             { return(DO); }
"double"         { return(DOUBLE); }
"else"           { return(ELSE); }
"enum"           { return(ENUM); }
"extern"         { return(EXTERN); }
"float"          { return(FLOAT); }
"for"            { return(FOR); }
"goto"           { return(GOTO); }
"if"             { return(IF); }
"inline"         { return(INLINE); }
"int"            { return(INT); }
"long"           { return(LONG); }
"register"       { return(REGISTER); }
"restrict"       { return(RESTRICT); }
"return"         { return(RETURN); }
"short"          { return(SHORT); }
"signed"         { return(SIGNED); }
"sizeof"         { return(SIZEOF); }
"static"         { return(STATIC); }
"struct"         { return(STRUCT); }
"switch"         { return(SWITCH); }
"typedef"        { return(TYPEDEF); }
"union"          { return(UNION); }
"unsigned"       { return(UNSIGNED); }
"void"           { return(VOID); }
"volatile"       { return(VOLATILE); }
"while"          { return(WHILE); }
"_Alignas"                              { return ALIGNAS; }
"_Alignof"                              { return ALIGNOF; }
"_Atomic"                               { return ATOMIC; }
"_Bool"                                 { return BOOL; }
"_Complex"                              { return COMPLEX; }
"_Generic"                              { return GENERIC; }
"_Imaginary"                            { return IMAGINARY; }
"_Noreturn"                             { return NORETURN; }
"_Static_assert"                        { return STATIC_ASSERT; }
"_Thread_local"                         { return THREAD_LOCAL; }
"__func__"                              { return FUNC_NAME; }

{L}{A}*                                  { return check_type(); }

{HP}{H}+{IS}?                            { return I_CONSTANT; }
{NZ}{D}*{IS}?                            { return I_CONSTANT; }
"0"{O}*{IS}?                             { return I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"              { return I_CONSTANT; }

{D}+{E}{FS}?                             { return F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?                     { return F_CONSTANT; }
{D}+"."{E}?{FS}?                         { return F_CONSTANT; }
{HP}{H}+{P}{FS}?                         { return F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?                  { return F_CONSTANT; }
{HP}{H}+"."{P}{FS}?                      { return F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+        { return STRING_LITERAL; }

"..."          { return ELLIPSIS; }
">>="          { return RIGHT_ASSIGN; }
"<<="          { return LEFT_ASSIGN; }
"+="           { return ADD_ASSIGN; }
"-="           { return SUB_ASSIGN; }
"*="           { return MUL_ASSIGN; }
"/="           { return DIV_ASSIGN; }
"%="           { return MOD_ASSIGN; }
"&="           { return AND_ASSIGN; }
"^="           { return XOR_ASSIGN; }
"|="           { return OR_ASSIGN; }
">>"           { return RIGHT_OP; }
"<<"           { return LEFT_OP; }
"++"           { return INC_OP; }
"--"           { return DEC_OP; }
"->"           { return PTR_OP; }
"&&"           { return AND_OP; }
"||"           { return OR_OP; }
"<="           { return LE_OP; }
">="           { return GE_OP; }
"=="           { return EQ_OP; }
"!="           { return NE_OP; }


";"            { return ';'; }
("{"|"<%")     { return '{'; }
("}"|"%>")     { return '}'; }
","            { return ','; }
":"            { return ':'; }
"="            { return '='; }
"("            { return '('; }
")"            { return ')'; }
("["|"<:")     { return '['; }
("]"|":>")     { return ']'; }
"."            { return '.'; }
"&"            { return '&'; }
"!"            { return '!'; }
"~"            { return '~'; }
"-"            { return '-'; }
"+"            { return '+'; }
"*"            { return '*'; }
"/"            { return '/'; }
"%"            { return '%'; }
"<"            { return '<'; }
">"            { return '>'; }
"^"            { return '^'; }
"|"            { return '|'; }
"?"            { return '?'; }

{WS}+          { 
  /* whitespace separates tokens */
}

        
  /** 
  .......................................
  ........ Some non-C grammars ..........
  .......................................
  **/

^[ \t]*#[ \t]+[0-9]+[ \t]+\"([^"\\\n]|{ES})*\".*  { 
  reset_file_line (ast, yytext);
}

"__attribute__"{WS}*\(                            {
  /* 
  .. Fixme :  attributes should be rather,
  .. forwarded to the compiler 
  */
  ignore_attribute (ast);
}

  /** 
  .......................................
  ....... End of non-C grammars ........
  .......................................
  **/


.                                                  { 
  /*
  .. Applies for each characters. 
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
  */

  static void ignore_attribute(_Ast * ast)
  {
    int lineno = yylineno;
    char c = 1, nscope = 1;
    while (nscope && c) {
      c = input();
      if (c == ')') 
        nscope--;
      else if (c == '(')
        nscope++;
    }
  
    if(nscope) { 
      yylineno = lineno;
      yyerror (ast, "__attribute__ not closed");
    }
  }

  
  /*
  .. This function is used to reset filename, line number 
  */
  static void reset_file_line(_Ast * ast, const char * str) {
    
    char * s = strchr (str, '#') + 1;
    yylineno = atoi(s) - 1;

    char source[_HMESH_PARSER_FILENAME_MAX_ + 1];
    s = strchr (str, '"') + 1;
    strncpy (source, s, _HMESH_PARSER_FILENAME_MAX_);
    source[_HMESH_PARSER_FILENAME_MAX_] = '\0';
    char * end = strchr(source, '"');
    if(end) 
      *end = '\0';
    AstResetSource(ast, source);
  }
  
  /* 
  .. This function 'consumes' (skips) comments 
  */
  static void comment(_Ast * ast)
  {
    int c, lineno = yylineno;
    /* 
    .. input() is the default function to read one
    .. character from yyin 
    */
    while ((c = input()) != 0)
      if (c == '*')
        {
          while ((c = input()) == '*')
            ;
          if (c == '/')
            return;
          if (c == 0)
            break;
        }
    yylineno = lineno;
    yyerror(ast, "unterminated comment");
  }
  
  /* which type of identifier ? */
  static int check_type(void)
  {
    switch (sym_type(yytext)) {
      case TYPEDEF_NAME:                /* previously defined */
        return TYPEDEF_NAME;
      case ENUMERATION_CONSTANT:        /* previously defined */
        return ENUMERATION_CONSTANT;
      default:                          /* includes undefined */
        return IDENTIFIER;
    }
  }
