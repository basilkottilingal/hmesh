/*
.. Convert "_parser.y" to "parser.y". "parser.y" is the same 
.. as "_parser.y", but create internal ast nodes each time it
.. encounters one. 
..  $ gcc -o rule-tree rule-tree.c -L./../ast -last && ./rule-tree  < _parser.y > parser.y
..  $ diff _parser.y parser.y
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../ast/memory.h"
#include "../ast/hash.h"

typedef struct _Rule {
  struct _Rule *** subrules;
  const char * name;
  int n;
  int flag;
} _Rule;

enum RuleType {
  RULE_PARENT = 0,      /* Internal/root node  */
  RULE_TOKEN = 1,       /* Token identifier    */
  RULE_SOURCE_CODE = 2, /* { ... something.. } */
  RULE_TRAVERSED = 4,   /* flag set during traversal */
  RULE_DOESNT_HAVE_TYPEDEF_NAME = 8, 
  RULE_DOESNT_HAVE_ANY_IDENTIFIER = 16,
  RULE_MAY_HAVE_EITHER = 32
};

_HashTable * Table = NULL;
_Rule ** Rules = NULL;
int nrules = 0;

static void init () {
  Table = hash_table_init (9);   //2^9 buckets
  fprintf(stderr, "Table created "); fflush(stderr);
  Rules = ast_allocate_general (512 * sizeof(_Rule *)); //max of 512 rules expected including 
  fprintf(stderr, "Rules pool allocated "); fflush(stderr);
  nrules = 0;
  assert ( Table && Rules );
}

static void destroy () {
  hash_table_free(Table);
  ast_deallocate_all();
}

static _Rule * rule_allocate (const char * name) {

  assert (nrules < 512);
  if ( name[0] == '{' ) {
fprintf(stderr, "-D-"); fflush(stderr);
    _Rule * rule = Rules[ nrules++ ] = ast_allocate_general (sizeof (_Rule));
fprintf(stderr, "-E-"); fflush(stderr);
    rule->name = ast_strdup (name);
fprintf(stderr, "-F-"); fflush(stderr);
    rule->flag = RULE_SOURCE_CODE ;
fprintf(stderr, "-G-"); fflush(stderr);
    return rule;
  }

  _HashNode * h =  hash_lookup (Table, name);
  if(h)
    return Rules[h->symbol];

  h = hash_insert (Table, name, nrules);
  assert(h);
  _Rule * rule = Rules[ nrules++ ] = ast_allocate_general (sizeof (_Rule));
  rule->name = h->key;
  rule->flag = RULE_TOKEN ;
  return rule;  
}

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

/*
.. Return the next identifier token (rule name) or mid-rule/end-rule section.
.. Return Null otherwise
*/
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

  /*
  .. an identifier (rule name)
  */
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
    int scope = 0;
    do {
      *strindex++ = *c;
      if ( *c == '{' )
        ++scope;
      else if (*c == '}') 
        --scope;
    } while ( (*c = getchar()) != EOF && scope );
    *strindex++ = '\0';
    //*c = getchar();
    return id;
  }

  /*
  .. Single character lexer token inside single quote
  */
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

  _Rule * rule, * chain [32] = {NULL}, ** subrules [32] = {NULL}; 
  const char * id = NULL, * child [32], * parent = NULL;

  strpool_reset ();
  int separator = getchar();

  while ( (parent = identifier ( &separator )) != NULL ) { 
  
    rule = rule_allocate (parent);
    assert(rule && !rule->subrules);
    rule->flag = RULE_PARENT;

    printf("\n\n%s\n  :", parent); 

    assert( !identifier( &separator ) && separator == ':' );
    strindex = strpool + strlen (parent) + 1;
 
    int  k = 0;
    do {
      separator = getchar();
      int n = 0, m = 0;
      for ( id = identifier( &separator ); id; id = identifier( &separator ) ) {
        chain [m] = rule_allocate ( child [m] = id );
        m++;

        if ( id[0] != '{' )
          n++; 

        //else {
        //  if( !strcmp (id, "error") ) {
        //    child[m++] = strindex;
        //    sprintf( strindex, "{ /*$$ = ast_node_new (YYSYMBOL_YYerror, 0); */ }" ); 
        //    strindex += strlen (strindex) + 1;
        //  }
        //}
      }
      assert(n); // m>=n

      /* Create a new sub rule from chain */
      subrules[k] = ast_allocate_general ( (m+2) * sizeof (_Rule *));
      memcpy ( subrules[k], chain, m * sizeof (_Rule *) );
      subrules[k][m] = subrules[k][m+1] = NULL;
      ++k;

      /*
      .. Define an end-rule section, say ERS, where you create an internal node 
      .. for this rule, and connect it with it's children. 
      */
      child[m] = strindex;
      sprintf( strindex, 
          "{\n      $$ = ast_node_new (ast, YYSYMBOL_%s, %d);"
          "\n      ast_node_children($$, %d",
          parent, n, n );
      strindex += strlen (strindex);
      for(int i = 0; i < m; ++i) 
        if (child[i][0] != '{' ) {
          sprintf( strindex, ", $%d", i+1); 
          strindex += strlen (strindex);
        }
      sprintf( strindex, ");\n    }"); 
      strindex += strlen (strindex) + 1;


      if( child[m-1][0] != '{' ) {  
        /* 
        .. If there is no existing end-rule section, then ERS 
        .. is taken as the end rule section.
        */
        subrules[k-1][m] = rule_allocate ( child[m] );
        m++;
      }
      else if ((m>2) && (child[m-2][0] == '{') ) {
        if (child[m-3][0] != '{') {
          /* 
          .. If there are exactly two end-rule sections already
          .. there, then insert ERS in between them.
          .. It's designed such a way that ERS can be inserted 
          .. before or after a pre-defined end-rule section.
          .. How? Ex 1:  rule: ID {<PRE>} {      };
          ..      Ex 2:  rule: ID {     } {<POST>};
          ..      Ex 3:  rule: ID {<PRE>} {<POST>};
          .. So things in PRE will preceed ERS and things
          .. in POST will come after ERS
          */ 
          const char * temp = child[m];
          child[m] = child[m-1]; 
          child[m-1] = temp;
          m++;
          /* 
          .. combine all the end-rules into a single one 
          */
          for (int k=m-2; k>=m-3; k--) {
            int l = strlen (child[k]);
            char * str = (char *) child[k];
            str[l-1] = ' ';  
          }
          child[m-2]++, child[m-1]++;

          sprintf(strindex, "%s%s%s", child[m-3], child[m-2], child[m-1]);
          //nrules -= 2;
          subrules[k-1][m-3]->name = ast_strdup ( strindex );
          subrules[k-1][m-2] = NULL;
          
        }
      }
      /*
      .. In all other cases ERS won't be added. 
      */
     
      /*
      .. Reprint the rule with appropriate end-rule
      */ 
      for(int i=0; i < m; ++i)
        printf(" %s", child[i]);
      printf("\n  %c", separator);

      assert( separator == ';' || separator == '|'  );


    } while ( separator != ';' && separator != EOF ); 

    separator = getchar();  
    strpool_reset ();

    /* Create a set of subrules for this rule 'rule' */
    rule->subrules = ast_allocate_general ( (k+1) * sizeof (_Rule **));
    memcpy (rule->subrules, subrules, k * sizeof (_Rule **));
    rule->n = k;
    rule->subrules[k] = NULL;
  }

  assert ( separator == '%' );
  assert ( getchar() == '%' );
  printf("\n\n%%%%");

}

void print () {
  for (int i=0; i<nrules; ++i) {
    _Rule * rule = Rules[i];
    if(rule && rule->subrules) {
      _Rule *** subrule = rule->subrules, ** chain = NULL;
      
      fprintf(stderr, "\n%s\n  :", rule->name); fflush(stderr);
      int k = rule->n;
      while( (chain = *subrule++) ){
        int n = 0;
        k--;
        while( chain[n] ) {
          fprintf(stderr, "  %s", chain[n]->name);
          ++n;
        }
        fprintf(stderr, "\n  %c", k ? '|' : ';' );
      }
    }
  }
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
  .. 
  */
  print ();

  while ( (c = getchar ()) != EOF ) 
    putchar (c); 

  destroy();
}
