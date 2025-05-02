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
  #include "parser.h"

  #include <ast.h>

  /* prints grammar violation message. defined in parser.y*/
  extern void yyerror(const char *);  

  /* returns type from symbol table */
  extern int sym_type(const char *);  

  /* with no symbol table, fake it */
  #define sym_type(identifier) IDENTIFIER 

  /* Ignore multi-line comments */
  static void comment(void);

  /* check_type of the identifier */
  static int check_type(void);

  /* Ignore __attribute__ () */
  static void ignore_attribute(void);

  /* 
  .. set location of parser whenever tokens like 
  ..   # 5 "header.h"
  .. are encountered
  */
  static void reset_file_line(char *);
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
.. */

%%

"/*"            { comment(); }
"//".*          { /* consume //-comment */ }

"auto"					{ return(AUTO); }
"break"					{ return(BREAK); }
"case"					{ return(CASE); }
"char"					{ return(CHAR); }
"const"					{ return(CONST); }
"continue"		  { return(CONTINUE); }
"default"				{ return(DEFAULT); }
"do"					  { return(DO); }
"double"				{ return(DOUBLE); }
"else"					{ return(ELSE); }
"enum"					{ return(ENUM); }
"extern"				{ return(EXTERN); }
"float"					{ return(FLOAT); }
"for"					  { return(FOR); }
"goto"					{ return(GOTO); }
"if"				 	  { return(IF); }
"inline"				{ return(INLINE); }
"int"					  { return(INT); }
"long"					{ return(LONG); }
"register"		  { return(REGISTER); }
"restrict"		  { return(RESTRICT); }
"return"				{ return(RETURN); }
"short"					{ return(SHORT); }
"signed"				{ return(SIGNED); }
"sizeof"				{ return(SIZEOF); }
"static"				{ return(STATIC); }
"struct"				{ return(STRUCT); }
"switch"				{ return(SWITCH); }
"typedef"				{ return(TYPEDEF); }
"union"					{ return(UNION); }
"unsigned"		  { return(UNSIGNED); }
"void"					{ return(VOID); }
"volatile"		  { return(VOLATILE); }
"while"					{ return(WHILE); }
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

{L}{A}*					                        { return check_type(); }

{HP}{H}+{IS}?				                    { return I_CONSTANT; }
{NZ}{D}*{IS}?				                    { return I_CONSTANT; }
"0"{O}*{IS}?				                    { return I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"		          { return I_CONSTANT; }

{D}+{E}{FS}?				                    { return F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?			              { return F_CONSTANT; }
{D}+"."{E}?{FS}?			                  { return F_CONSTANT; }
{HP}{H}+{P}{FS}?			                  { return F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?			            { return F_CONSTANT; }
{HP}{H}+"."{P}{FS}?			                { return F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	      { return STRING_LITERAL; }

"..."					{ return ELLIPSIS; }
">>="					{ return RIGHT_ASSIGN; }
"<<="					{ return LEFT_ASSIGN; }
"+="					{ return ADD_ASSIGN; }
"-="					{ return SUB_ASSIGN; }
"*="					{ return MUL_ASSIGN; }
"/="					{ return DIV_ASSIGN; }
"%="					{ return MOD_ASSIGN; }
"&="					{ return AND_ASSIGN; }
"^="					{ return XOR_ASSIGN; }
"|="					{ return OR_ASSIGN; }
">>"					{ return RIGHT_OP; }
"<<"					{ return LEFT_OP; }
"++"					{ return INC_OP; }
"--"					{ return DEC_OP; }
"->"					{ return PTR_OP; }
"&&"					{ return AND_OP; }
"||"					{ return OR_OP; }
"<="					{ return LE_OP; }
">="					{ return GE_OP; }
"=="					{ return EQ_OP; }
"!="					{ return NE_OP; }


";"					  { return ';'; }
("{"|"<%")	 	{ return '{'; }
("}"|"%>")	  { return '}'; }
","					  { return ','; }
":"					  { return ':'; }
"="					  { return '='; }
"("					  { return '('; }
")"					  { return ')'; }
("["|"<:")		{ return '['; }
("]"|":>")		{ return ']'; }
"."					  { return '.'; }
"&"					  { return '&'; }
"!"					  { return '!'; }
"~"					  { return '~'; }
"-"					  { return '-'; }
"+"					  { return '+'; }
"*"					  { return '*'; }
"/"					  { return '/'; }
"%"					  { return '%'; }
"<"					  { return '<'; }
">"					  { return '>'; }
"^"					  { return '^'; }
"|"					  { return '|'; }
"?"					  { return '?'; }

{WS}+					{ /* whitespace separates tokens */ }

        
  /** 
  .......................................
  ........ Some non-C grammars ..........
  .......................................
  **/

^[ \t]*#[ \t]+[0-9]+[ \t]+\"([^"\\\n]|{ES})*\".*  { 
  reset_file_line(yytext);
}

"__attribute__"{WS}*\(                            {
  ignore_attribute ();
}

  /** 
  .......................................
  ....... End of non-C grammars ........
  .......................................
  **/


.					                                        { 
  /* discard bad characters. 
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
static void ignore_attribute(void)
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
    yyerror("__attribute__ not closed");
  }
}

/*
.. This function is used to reset filename, line number 
*/
static void reset_file_line(char * str) {
  char * s = strchr (str, '#') + 1;
  yylineno = atoi(s) - 1;
  /* 
  .. Fixme: file name not yet set. Need a
  .. parse/lex parameter or a global parameter
  .. which store file name 
  */  
}

/* 
.. This function 'consumes' (skips) comments 
*/
static void comment(void)
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
  yyerror("unterminated comment");
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
