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
  RULE_HAVE_TYPEDEF_NAME = 8, 
  RULE_HAVE_IDENTIFIER = 16
};

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

extern
void print () {
  for (int i=0; i<nrules; ++i) {
    _Rule * rule = Rules[i];
    if(rule && rule->subrules) {
      _Rule *** subrule = rule->subrules, ** chain = NULL;
      
      fprintf(stderr, "\n\n%s[%d]\n  :", rule->name, rule->flag); fflush(stderr);
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
  
  
/*
.. traverse() : DFS search, max rule tree depth is 128
*/

struct RuleStack {
  _Rule * rule, * parent;
  int i, j;
} stack [ 128 ] ;

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
    while ( child = subrules[*i][(*j)++] ) {
      /* 
      .. Skip source code like {..something..}, and nodes
      .. already traversed
      */
      if( ( child->flag & 3) != RULE_SOURCE_CODE  && 
          ( child->flag & RULE_TRAVERSED ) == 0  ) 
      {
        stack[++(*level)] = 
          (struct RuleStack) {
          .parent = rule,
          .rule = child,
          .i = 0, .j = 0
        };
        child->flag |= RULE_TRAVERSED;
        return 1;
      }
      else if (child->flag & RULE_TRAVERSED) {
        rule->flag |= 
          child->flag & (RULE_HAVE_TYPEDEF_NAME | RULE_HAVE_IDENTIFIER);
      }
    }
    (*i)++;
    *j = 0;
  }
  return 0;
}

static inline
void pop (int * level) {
  (*level)--;
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

  /*
  .. DFS, Post Order.
  .. Each instance, you go back up the chain, 
  .. you can reduce properties, Ex : 
  ..    f(parent) = max { f(child) | each 'child' of 'parent' }
  */

  while (level >= 0) {
    /* Go down till the max depth. */
    while ( push (&level) ) {};

    /* Do something 
    fprintf(stderr, "\n");
    for(int i=0; i<level; ++i)
      fprintf(stderr,"  ");
    */
    _Rule * rule = stack[level].rule, 
      * parent = stack[level].parent; 
    if(parent) {
      parent->flag |= 
        rule->flag & (RULE_HAVE_TYPEDEF_NAME | RULE_HAVE_IDENTIFIER);
    }
    fprintf(stderr, "\n%s[%d][t%d][i%d]", 
      rule->name, level, rule->flag & RULE_HAVE_TYPEDEF_NAME,
      rule->flag & RULE_HAVE_IDENTIFIER );

    /* Pop */
    pop( &level );
  }

}
