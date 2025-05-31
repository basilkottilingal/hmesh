/*
.. Convert "_parser.y" to "parser.y". "parser.y" is the same 
.. as "_parser.y", but create internal ast nodes each time it
.. encounters one. 
..  $ gcc -o convert convert.c && ./convert  < _parser.y > parser.y
..  $ diff _parser.y parser.y
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
.. Single character tokens 
*/

const char * tokens[] = {
  "SEMICOLON", "LBRACE", "RBRACE", "COMMA", "COLON", "EQUAL",
  "LPARANTHESIS", "RPARANTHESIS", "LBRACKET", "RBRACKET", "DOT",
  "AMPERSAND", "NOT", "TILDE", "MINUS", "PLUS", "STAR", "SLASH",
  "PERCENT", "L_T", "G_T", "CARET", "PIPE", "QUESTION"
};

static inline const char *get_token(int c) {
  switch (c) {
    case ';': return tokens[0];  // SEMICOLON
    case '{': return tokens[1];  // LBRACE
    case '}': return tokens[2];  // RBRACE
    case ',': return tokens[3];  // COMMA
    case ':': return tokens[4];  // COLON
    case '=': return tokens[5];  // EQUAL
    case '(': return tokens[6];  // LPARANTHESIS
    case ')': return tokens[7];  // RPARANTHESIS
    case '[': return tokens[8];  // LBRACKET
    case ']': return tokens[9];  // RBRACKET
    case '.': return tokens[10]; // DOT
    case '&': return tokens[11]; // AMPERSAND
    case '!': return tokens[12]; // NOT
    case '~': return tokens[13]; // TILDE
    case '-': return tokens[14]; // MINUS
    case '+': return tokens[15]; // PLUS
    case '*': return tokens[16]; // STAR
    case '/': return tokens[17]; // SLASH
    case '%': return tokens[18]; // PERCENT
    case '<': return tokens[19]; // L_T
    case '>': return tokens[20]; // G_T
    case '^': return tokens[21]; // CARET
    case '|': return tokens[22]; // PIPE
    case '?': return tokens[23]; // QUESTION
    default : return NULL;
  }
  return NULL;
}

/*
.. fixme : I don't know if digits are allowed for
.. rule (symbol) identifiers in bison
*/
#define is_token(c) ( (c == '_') || \
  (c >= 'a' && c <= 'z')  || (c >= 'A' && c <= 'Z') )

/* 
.. Limitations : 
..  4096 chars per rule.
..  16 symbols per rule 
*/
char strpool[4096];
char * strindex = NULL;

static inline 
void strpool_reset () {
  strindex = strpool;
}

const char * identifier (int * c) {

  /* 
  .. remove all whitespaces and comment 
  */
  while ( strchr (" /\n\t", *c) ) {
    int p = 0;
    if (*c == '/') {  
      assert( getchar() == '*' );
      while ( (*c = getchar () ) != EOF ) {
        if ( *c == '/' && p == '*')
          break;
        p = *c;
      }
      *c = getchar();
    }
    else {
      while ( (*c = getchar()) != EOF && strchr (" \n\t", *c) ) {
      }
    }
  }

  const char * id = strindex;

  if( is_token (*c) ) {
    do {
      *strindex++ = (char) *c;
    } while ( (*c = getchar()) != EOF && is_token (*c) );
    *strindex++ = '\0';
    return id;
  }

  if ( *c == '{' ) {
    /*
    .. warning : no brace expected inside string/char const
    */
    int scope = 1, p = '\0';
    do {
      *strindex++ = *c;
      if ( *c == '{' )
        ++scope;
      else if  (*c == '}');
        --scope;
    } while ( (*c = getchar()) != EOF && scope );
    *strindex++ = '}';
    *strindex++ = '\0';
    *c = getchar();
    return id;
  }

  if ( *c == '\'' ) {
    const char * token = get_token (getchar());
    assert(token);
    assert(getchar() == '\'');
    *c = getchar();
    return token;
  }

  return NULL;
} 


void read_rules ( void ) {

  const char * child [32] = {NULL}, * csource [32] = {NULL}, 
    * id = NULL, * parent;

  strpool_reset ();
  int separator = getchar();

  while ( (parent = identifier ( &separator )) != NULL ) { 
    printf("\n\n%s\n  :", parent); 

    assert( !identifier( &separator ) && separator == ':' );
    strindex = strpool + strlen (parent) + 1;
 
    int k = 0;
    int error = 0;
    do {
      separator = getchar();
      int n = -1;
      for ( id = identifier( &separator ); id; id = identifier( &separator ) ) {
        if (id[0] == '{') {
          assert( child[n] );
          csource[n] = id;
        }
        else {
          child[++n] = id;
          csource[n] = NULL;
          if( !strcmp (id, "error") ) 
            error = n + 1;
        }
      }

      if(!csource[n]) {
        csource[n] = strindex;
        /*
        if(error) {
          sprintf( strindex, "{ $$ = ast_node_new (YYSYMBOL_YYerror, 0); }" ); 
          strindex += strlen (strindex) + 1;
        }
        */
        sprintf( strindex, 
          "{\n      $$ = ast_node_new (YYSYMBOL_%s, %d);"
          "\n      ast_node_children($$",
          parent, n+1);
        strindex += strlen (strindex);
        for(int i=0; i <= n; ++i) {
          sprintf( strindex, ", $%d", i+1); 
          strindex += strlen (strindex);
        }
        sprintf( strindex, ");\n    }"); 
        strindex += strlen (strindex) + 1;
      }
      
      for(int i=0; i <= n; ++i)
        printf(" %s %s", child[i], csource[i] ? csource[i] : "");
      printf("\n  %c", separator);

      assert( (separator == ';' || separator == '|' ) && ( ++n > 0 ) );
      k++;
    } while ( separator != ';' && separator != EOF ); 

    separator = getchar();  
    strpool_reset ();
  }

  assert ( separator == '%' );
  assert ( getchar() == '%' );
  printf("\n\n%%%%");

}

int main () {
  int c, p = 0, pp = 0;

  
  while ( (c = getchar ()) != EOF ) {
    if( pp = '\n' && c == '%' && p == '%' ) {
      putchar(c);
      break;  
    }
    putchar(c);
    pp = p;
    p = c;
  }

  /*
  .. Fails when first '\n%%' not found
  */
  assert ( c == '%' );

  /*
  .. Read all grammars that falls in %% and %%
  */
  read_rules ();

  while ( (c = getchar ()) != EOF ) 
    putchar (c); 
}
