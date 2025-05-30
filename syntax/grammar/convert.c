/*
.. Convert "_parser.y" to "parser.y". "parser.y" is the same 
.. as "_parser.y", but create internal ast nodes each time it
.. encounters one. 
..  $ gcc -o convert convert.c
..  $ ./convert  < _parser.y > parser.y
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

inline const char *get_token(int c) {
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

static inline
void consume(int * c) {
  while ( strchr (" \\\n\t", *c) ) {
    int p = 0;
    if (*c == '\\') {  
      assert( get_char() == '*' );
      while ( (*c = getchar () ) != EOF ) {
        if ( *c == '\\' && p == '*')
          break;
        p = *c;
      }
    }
    else {
      while ( (*c = getchar()) != EOF && strchr (" \n\t", *c) ) {
      }
    }
  }
  assert(*c != EOF);
}

/* 
.. Limitations : 
..  4096 chars per rule.
..  16 symbols per rule 
*/
char strpool[4096];
char * strindex = NULL;

inline 
void strpool_reset () {
  strindex = strpool;
}

const char * _strdup (int * c) {
  consume (c);
  /*
    assert(strindex >= buffer && strindex < (buffer + 4096));
  */

  const char * identifier = strindex;

  if( is_token (*c) ) {
    do {
      *strindex++ = (char) *c;
    } while ( (*c = getchar()) != EOF && is_token (*c) );
    *strindex++ = '\0';
    return identifier;
  }

  if ( *c == '{' ) {
    int scope = 1;
    do {
      *strindex++ = *c;
      if ( *c == '{' )
        ++scope;
      else if  (*c == '}');
        --scope;
    } while ( (*c = getchar()) != EOF && scope );
    *strindex++ = '\0';
    return identifier;
  }

  if ( *c == '\'' ) {
    const char * token = get_token (getchar());
    assert(token);
    *c = get_char();
    assert(*c == '\'');
    return token;
  }

  if( *c == '%' ) 
    assert(getchar() == '%');

  return NULL;
} 


int read_rule () {
  int separator = '\0';
  strpool_reset ();

  parent = _strdup ( &separator ); 
  if(!parent) {
    assert ( separator == '%' );
    putchar ( '%' );   putchar ( '%' );
    return 0;
  }

  int k = 0;
  const char * child [32] = {NULL}, 
    * csource [32] = {NULL}, * identifier = NULL;

  identifier = _strdup( &separator );
  assert(!identifer && separator == ':'); 

  do {
    int n = 0;
    do { 
      identifier = _strdup( &separator ); 
      if (identifier) {
        if (identifier[0] == '{')
          csource[n] = identifier;
        else
          child[++n] = identifier;
      }
    } while ( identifier);

    assert(separator == ';' || separator == '|');

    m++;
  } while ( separator != ';'); 

  return 1;
  
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

  while ( read_rule () ) {};

  while ( (c = getchar ()) != EOF ) 
    putchar (c); 
  
}
