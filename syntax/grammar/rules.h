#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../ast/memory.h"
#include "../ast/hash.h"

/* 
.. Limitations : 
..  4096 chars per rule.
..  16 symbols per rule 
*/

char strpool[4096];
char * strindex = NULL;

void strpool_reset () {
  strindex = strpool;
}

#define STRING_APPEND(str, buff, ...)          \
  do {                                         \
    int _n_ = sprintf(str,buff,##__VA_ARGS__); \
    str += _n_;                                \
  } while(0);

typedef struct _Rule {
  struct _Rule *** subrules;
  const char * name;
  int n;
  int flag;
} _Rule;

enum RuleType {
  RULE_PARENT = 0,       /* Internal/root node  */
  RULE_TOKEN = 1,        /* Token identifier    */
  RULE_SOURCE_CODE = 2,  /* { ... something.. } */
  RULE_NOT_PLACE_HOLDER = 4, /* A parent rule with atleast one token as a leaf node */
  RULE_TRAVERSED = 8,    /* flag set during traversal */
  RULE_HAVE_TYPEDEF_NAME = 16, 
  RULE_HAVE_IDENTIFIER = 32
};

#define NOT_PLACE_HOLDER(f) ( (( (f) & 3 ) == RULE_TOKEN) ? RULE_NOT_PLACE_HOLDER \
  : (( (f) & 3 ) == RULE_SOURCE_CODE) ? 0 : ( (f) & RULE_NOT_PLACE_HOLDER ) )

/*
.. HashTable for rule names, and
.. memory pool for rules
*/
_HashTable * Table = NULL;
_Rule ** Rules = NULL;
static int nrules = 0;

/*
.. Initialize (a) hash table with 2^9 = 512 slots, and (b) memory pool. 
.. NOTE : 512 max rule names expected.
*/
extern
void init () {
  Table = hash_table_init (9); 
  fprintf(stderr, "Table created "); 
  Rules = ast_allocate_general (512 * sizeof(_Rule *)); 
  fprintf(stderr, "Rules pool allocated "); 
  nrules = 0;
  assert ( Table && Rules );
}

extern
void destroy () {
  hash_table_free(Table);
  ast_deallocate_all();
}

static inline
_Rule * rule_new () {
  assert ( nrules < 512 );
  return ( Rules[ nrules++ ] = ast_allocate_general (sizeof (_Rule)) );
}

extern
_Rule * rule_allocate (const char * name) {

  /*
  .. Mid/end section rules like {..something..}
  .. NOTE : WARNING: Mid section rules are not 
  .. recommended as they may confuse bison and produce 
  .. unexpected behavior.
  */
  if ( name[0] == '{' ) {
    _Rule * rule = rule_new() ;
    rule->name = ast_strdup (name);
    rule->flag = RULE_SOURCE_CODE ;
    return rule;
  }

  /* 
  .. See if the rule name is already created 
  */
  _HashNode * h =  hash_lookup (Table, name);
  if(h)
    return Rules[h->symbol];

  /*
  .. Create a new rule name if not found
  */
  h = hash_insert (Table, name, nrules);
  assert(h);
  _Rule * rule = rule_new() ;
  rule->name = h->key;
  rule->flag = RULE_TOKEN ;
  return rule;
 
}

static
const char * endrule ( const char * parent, _Rule ** chain ) {
  int n = 0, m = 0, k = 0;
  _Rule * child = NULL;
  while( ( child = chain[n++] ) ) {
    if ( NOT_PLACE_HOLDER (child->flag) )
      k++;
    if ( ( child->flag & 3 ) == RULE_SOURCE_CODE )
      m++;
    else
      m = 0;
  }
  
  if ( m == 1 || k == 0 )
    return NULL;

  assert ( m == 0 || m == 2 );

  char * pool = strpool;
    
  STRING_APPEND ( pool, "%s", m == 2 ? chain[n-3]->name : "{" );
  if( m == 2 )
    --pool;
  STRING_APPEND ( pool, "\n      $$ = ast_node_new (ast, YYSYMBOL_%s, %d);", parent, k );
  STRING_APPEND ( pool, "\n      ast_node_children($$, %d", k );
  n = 0;
  while( ( child = chain[n++] ) ) {
    if ( NOT_PLACE_HOLDER (child->flag) )
      STRING_APPEND ( pool, ", $%d", n ); 
  }
  STRING_APPEND ( pool, ");%s", m == 2 ? chain[n-2]->name + 1 : "\n    }");

  if( m == 2 )
    chain[n-3] = NULL;

  return strpool;
}

extern
void print () {
  for (int i=0; i<nrules; ++i) {
    _Rule * rule = Rules[i];
    if(rule && rule->subrules) {
      _Rule *** subrule = rule->subrules, ** chain = NULL;
      
      printf("\n\n%s /*%d*/\n  :", rule->name, NOT_PLACE_HOLDER (rule->flag)); 
      int k = rule->n;
      while( (chain = *subrule++) ){
        const char * end = endrule (rule->name, chain);
        int n = 0;
        k--;
        while( chain[n] ) {
          printf(" %s", chain[n]->name);
          ++n;
        }
        if ( end )
          printf(" %s", end);
        printf("\n  %c", k ? '|' : ';' );
      }
    }
  }
}
  
  
/*
.. traverse() : DFS search, max rule tree depth is 128
*/

struct RuleStack {
  _Rule * rule, * parent;
  int i, j;
} stack [ 128 ] ;

static inline
void pop (int * level) {
  _Rule * rule = stack [ *level ].rule,
    * parent = stack [ *level ].parent;
     
  if ( parent ) 
    parent->flag |= NOT_PLACE_HOLDER (rule->flag) | 
      (rule->flag & (RULE_HAVE_TYPEDEF_NAME | RULE_HAVE_IDENTIFIER));

  (*level)--;
}

static
int push (int * level) {
  _Rule * rule = stack[*level].rule;

  /* It's a token, i.e a terminal node */
  if ( (rule->flag & 3) != RULE_PARENT )
    return 0;

  assert( *level < 128 );
  int * i = & (stack[*level].i),
      * j = & (stack[*level].j);

  _Rule *** subrules = rule->subrules, * child;
  while ( subrules[*i] ) {
    while ( (child = subrules[*i][(*j)++]) ) {
      /* 
      .. Skip source code like {..something..}, and nodes
      .. already traversed
      */
      if( ( child->flag & 3) != RULE_SOURCE_CODE ) {
        stack[++(*level)] = 
          (struct RuleStack) {
          .parent = rule,
          .rule = child,
          .i = 0, .j = 0
        };
        if ( child->flag & RULE_TRAVERSED )
          pop (level);
        else {
          child->flag |= RULE_TRAVERSED;
          return 1;
        }
      }
      
    }
    (*i)++, *j = 0;
  }
  return 0;
}

extern
void traverse () {
  int level = 0;
  memset ( stack, 0, 128 * sizeof (struct RuleStack));
  for (int i=0; i<nrules; ++i) {
    Rules[i]->flag &= ~RULE_TRAVERSED;
  }

  _Rule * root = rule_allocate ("root"),
    * TYPEDEF_NAME = rule_allocate ("TYPEDEF_NAME"),
    * IDENTIFIER = rule_allocate ("IDENTIFIER");
  TYPEDEF_NAME->flag |= RULE_HAVE_TYPEDEF_NAME;
  IDENTIFIER->flag   |= RULE_HAVE_IDENTIFIER;
  assert( root && root->subrules );
  
  stack [0] = (struct RuleStack) {
    .parent = NULL,
    .rule = root,
    .i = 0, .j = 0
  };

  char * symbols = strpool, * pool = strpool; 
  STRING_APPEND(pool,"\n\nenum ast_symbols {");

  /*
  .. DFS, Post Order.
  .. Each instance, you go back up the chain, 
  .. you can reduce properties, Ex : 
  ..    f(parent) = max { f(child) | each 'child' of 'parent' }
  */

  while (level >= 0) {
    /* Go down till the max depth. */
    while ( push (&level) ) {};

    /* Do something */ 
    _Rule * rule = stack[level].rule; 
  
    STRING_APPEND(pool,"\n  sym_%s", rule->name);

    /* Pop */
    pop( &level );
  }

  STRING_APPEND(pool,"\n}");
  fprintf(stderr, "%s", symbols);

}

        //else {
        //  if( !strcmp (id, "error") ) {
        //    child[m++] = strindex;
        //    sprintf( strindex, "{ /*$$ = ast_node_new (YYSYMBOL_YYerror, 0); */ }" ); 
        //    strindex += strlen (strindex) + 1;
        //  }
        //}

      /*
    fprintf(stderr, "\n%s[l %d][t %d][i %d]", 
      rule->name, level, 
      rule->flag & RULE_HAVE_TYPEDEF_NAME ? 1 : 0,
      rule->flag & RULE_HAVE_IDENTIFIER ? 1 : 0);
      .. Define an end-rule section, say ERS, where you create an internal node 
      .. for this rule, and connect it with it's children. 
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
      */


        /* 
      if( child[m-1][0] != '{' ) {  
        .. If there is no existing end-rule section, then ERS 
        .. is taken as the end rule section.
        //subrules[k-1][m] = rule_allocate ( child[m] );
        //m++;
      }
      else if ((m>2) && (child[m-2][0] == '{') ) {
        if (child[m-3][0] != '{') {
          .. If there are exactly two end-rule sections already
          .. there, then insert ERS in between them.
          .. It's designed such a way that ERS can be inserted 
          .. before or after a pre-defined end-rule section.
          .. How? Ex 1:  rule: ID {<PRE>} {      };
          ..      Ex 2:  rule: ID {     } {<POST>};
          ..      Ex 3:  rule: ID {<PRE>} {<POST>};
          .. So things in PRE will preceed ERS and things
          .. in POST will come after ERS
          const char * temp = child[m];
          child[m] = child[m-1]; 
          child[m-1] = temp;
          m++;
          .. combine all the end-rules into a single one 
          for (int k=m-2; k>=m-3; k--) {
            int l = strlen (child[k]);
            char * str = (char *) child[k];
            str[l-1] = ' ';  
          }
          child[m-2]++, child[m-1]++;

          sprintf(strindex, "%s%s%s", child[m-3], child[m-2], child[m-1]);
          subrules[k-1][m-3]->name = ast_strdup ( strindex );
          subrules[k-1][m-2] = NULL;
          
        }
      }
      .. In all other cases ERS won't be added. 
      */
