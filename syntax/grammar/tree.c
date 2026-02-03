/*
.. "tree.c"
.. 
.. Create a new bison parser code from "./parser.y". The difference between the
.. source and target is that, the target ".y" file creates internal node each 
.. time bison parser detects a new rule.
.. 
.. Create a rule tree, in the process, whose root is "root".
.. 
.. Uses memory pool, string pool and  hash table in ../ast/libast.a
.. So make sure ../ast/libast.a is upto-date
..
.. $ gcc -o tree tree.c -L./../ast -last && ./tree  < parser.y > _parser.y
.. $ diff parser.y _parser.y
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rules.h"

/*
.. Single character tokens 
*/

const char * tokens[] = {
  "SEMICOLON", "LBRACE", "RBRACE", "COMMA", "COLON", "EQUAL",
  "LPARENTHESIS", "RPARENTHESIS", "LBRACKET", "RBRACKET", "DOT",
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
.. Return the next identifier token (rule name) or mid-rule/end-rule section.
.. Return Null otherwise
*/
_Rule * identifier (int * c) {

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

  /*
  .. Single character lexer token inside single quote
  */
  if ( *c == '\'' ) {
    const char * token = get_token (getchar());
    assert(token);
    assert(getchar() == '\'');
    *c = getchar();
    return rule_allocate (token) ;
  }

  char * id = strpool;

  if ( *c == '{' ) {
    int scope = 0, toggle = 0;
    do {
      *id++ = *c;
      if ( *c == '{' && !toggle )
        ++scope;
      else if (*c == '}' && !toggle ) 
        --scope;
      else if (*c == '"')
        toggle = !toggle;
    } while ( (*c = getchar()) != EOF && scope );
    *id++ = '\0';
    return rule_allocate (strpool) ;
  }

  /*
  .. an identifier (rule name)
  */
  if( is_token (*c) ) {
    do {
      *id++ = (char) *c;
    } while ( (*c = getchar()) != EOF && is_token (*c) );
    *id++ = '\0';
    return rule_allocate (strpool) ;
  }

  return NULL;
} 


void read_rules ( void ) {

  _Rule * rule, * chain [32] = {NULL}, ** subrules [32] = {NULL}; 

  int separator = getchar();

  while ( (rule = identifier ( &separator )) ) { 
    assert( !identifier( &separator ) && separator == ':' );
    assert( !rule->subrules );
    rule->flag = RULE_PARENT;
 
    int  k = 0;
    do {
      separator = getchar();
      int n = 0;
      while ( ( chain[n++] = identifier( &separator )) ) { };
      assert(n > 1);

      /* Create a new sub rule from chain */
      subrules[k] = ast_allocate_general ( n * sizeof (_Rule *));
      memcpy ( subrules[k], chain, n * sizeof (_Rule *) );
      ++k;

      assert( separator == ';' || separator == '|'  );

    } while ( separator != ';' && separator != EOF ); 

    separator = getchar();  

    /* Create a set of subrules for this rule 'rule' */
    rule->subrules = ast_allocate_general ( (k+1) * sizeof (_Rule **));
    memcpy (rule->subrules, subrules, k * sizeof (_Rule **));
    rule->n = k;
  }

  assert ( separator == '%' && getchar() == '%' );

}

int main () {
  init();

  int c, p = 0, pp = 0;

  while ( (c = getchar ()) != EOF ) {
    if( ( pp = '\n' && c == '%' ) &&  p == '%' ) {
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

  /*
  .. Traverse through the tree. DFS post-order (for reduction)
  .. Print Grammar with appropriate end-rule section 
  */
  for (int i=0; i<3; ++i)
    traverse ();
  print ();
  printf("\n\n%%%%");

  /*
  .. flush the rest of parser.y to ../parser.y
  */
  while ( (c = getchar ()) != EOF ) 
    putchar (c); 

  destroy();
}
