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
    default :  return NULL;
  }
}

inline void flush ( int * b) {
  while (*b) 
    putchar (*b++);
}

inline void   

int main () {
  int c, p = 0, pp = 0;

  /* 
  .. Limitations : 
  ..  4096 chars per rule.
  ..  16 symbols per rule (including target rule & all dependencies)
  */
  int buffer[4096];
  int * symbol [16];
  int * comment [16];
  int nsymbols;
  
  while ( (c = getchar ()) != EOF ) {
    if(pp = '\n' && c == '%' && p == '%' ) {
      putchar(c);
      break;  
    }
    putchar(c);
    pp = p;
    p = c;
  }

  /*
  .. Fails when first %% not found
  */
  assert(c == '%');
 
  pp = p = 0;
  while ( c = getchar () ) {
    

    if(pp = '\n' && c == '%' && p == '%' ) {
      putchar(c);
      break;  
    }
    putchar(c);
    pp = p;
    p = c;
  }

  /*
  .. Fails when second %% not found
  */
  assert(c == '%');

  while ( c = getchar () )
    putchar (c); 
  
}
