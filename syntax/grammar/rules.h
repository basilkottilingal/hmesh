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
  RULE_HAVE_type_specifier = 16, 
  RULE_HAVE_new_identifier = 32,
  RULE_MAY_START_WITH_type_specifier = 64,
  RULE_MAY_START_WITH_new_identifier = 128,
  RULE_SHOULD_END_WITH_SYMBOL_LOOKUP = 256,
  RULE_SHOULD_END_WITH_EXPECT_IDENTIFIER = 512
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
.. Initialize 
.. (a) hash table with 2^9 = 512 slots, and 
.. (b) memory pool. 
.. NOTE : 512 max rule names expected.
*/
extern
void init () {
  Table = hash_table_init (9); 
  Rules = ast_allocate_general (512 * sizeof(_Rule *)); 
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

  if ( name[0] == '{' ) {
    _Rule * rule = rule_new() ;
    rule->name = ast_strdup (name);
    rule->flag = RULE_SOURCE_CODE ;
    return rule;
  }

  /* 
  .. See if the rule name already exists.
  .. If not found, create a new rule with this name
  */
  _HashNode * h =  hash_lookup (Table, name);
  if(h)
    return Rules[h->symbol];

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

  return ast_strdup (strpool);
}

#define context_switch(_chain_,_i_) \
if (_i_) {\
  int f0 = _chain_[_i_-1]->flag, f1 = _chain_[_i_]->flag;\
  if ( f1 & RULE_MAY_START_WITH_type_specifier ) \
    printf("/*%s*/ ", \
      f0 & (RULE_HAVE_new_identifier | RULE_HAVE_type_specifier) ? "!" : "_"); \
}

extern
void print () {

  int a,b;

  for (int i=0; i<nrules; ++i) {
    _Rule * rule = Rules[i];
    if(rule && rule->subrules) {
      _Rule *** subrule = rule->subrules, ** chain = NULL;
      
      printf("\n\n%s /*%s%s- %s%s*/\n  :", rule->name, 
        rule->flag & RULE_HAVE_type_specifier ? "t" : "", 
        rule->flag & RULE_HAVE_new_identifier ? "i" : "",
        rule->flag & RULE_MAY_START_WITH_type_specifier ? "t" : "", 
        rule->flag & RULE_MAY_START_WITH_new_identifier ? "i" : "");
      while( (chain = *subrule++) ){
        const char * end = endrule (rule->name, chain);
        int n = 0;
        while( chain[n] ) {
          //printf(" %s", print_check_point(n,a,b));
          //context_switch (chain, n);
          printf(" %s", chain[n]->name);
          ++n;
        }
        if (rule->flag & RULE_SHOULD_END_WITH_SYMBOL_LOOKUP )
          printf(" /* ? */");
        //printf(" expect_identifier");
        //if ( end )
        //  printf(" %s", end);
        printf("\n  %c", *subrule ? '|' : ';' );
      }
    }
  }
}
 
_Rule ** insert ( _Rule *** subrule, int * j ) {
  _Rule ** chain = *subrule;
  int k = 0, m = (*j)++;
  while ( chain[k++] ) {};
  _Rule ** new_chain = *subrule = 
    ast_allocate_general( (k+1) * sizeof (_Rule *));
  memcpy (new_chain, chain, m*sizeof(_Rule *));
  memcpy (&new_chain[m+1], &chain[m+1], (k - m)*sizeof (_Rule *));
  return &new_chain[m];
}
  
/*
.. traverse() : DFS search, max rule tree depth is 128
*/

struct RuleStack {
  _Rule * rule, * parent;
  int i, j;
} stack [ 128 ] ;

static int traversal_number = 0;

static inline
void pop (int * level) {
  int l = *level, j = l ? (stack[l-1].j - 1) : 0, i = l ? stack[l-1].i : 0;
  _Rule * rule = stack [l].rule, * parent = stack [l].parent,
    * prev = (parent && j) ? parent->subrules[i][j-1] : NULL,
    * next = parent ? parent->subrules[i][j+1] :  NULL;
    
  /*
  .. Reduction of properties,
  */
  switch ( traversal_number ) {
    case 0 :
    case 1 :
      if ( parent ) { 
        parent->flag |= NOT_PLACE_HOLDER (rule->flag) |
          ( rule->flag & RULE_HAVE_type_specifier );
        if ( strcmp (parent->name, "type_name") )
          parent->flag |=  rule->flag & RULE_HAVE_new_identifier ;
        if ( !j ) 
          /* Applies only if 'rule' is the first in a chain */
          parent->flag |= (rule->flag & 
            (RULE_MAY_START_WITH_type_specifier | RULE_MAY_START_WITH_new_identifier) );
      }
      break;
    case 2 :
      if ( !prev )
        break; 
      int f0 = prev->flag, f1 = rule->flag;
      if ( !( (f1 & RULE_MAY_START_WITH_type_specifier) &&
            strcmp("type_specifier", prev->name ) ) )
        if ( f0 & (RULE_HAVE_new_identifier | RULE_HAVE_type_specifier) )
          prev->flag |= RULE_SHOULD_END_WITH_SYMBOL_LOOKUP;
      break;
    case 3:
      if ( parent && !next ) {
        /*
        .. add end-rule
        */
      }
      break;
  }

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
  for (int i=0; i<nrules; ++i)
    Rules[i]->flag &= ~RULE_TRAVERSED;
  
  _Rule * root = rule_allocate ("root");
  stack [0] = (struct RuleStack) {
    .parent = NULL,
    .rule = root,
    .i = 0, .j = 0
  };

  char * symbols = strpool, * pool = strpool;
  (void) symbols;
  (void) pool; 
    
  if ( !traversal_number ) { 
    rule_allocate ("type_specifier")->flag |= 
      ( RULE_HAVE_type_specifier | RULE_MAY_START_WITH_type_specifier );
    rule_allocate ("new_identifier")->flag |= 
      ( RULE_HAVE_new_identifier | RULE_MAY_START_WITH_new_identifier );
    STRING_APPEND(pool,"\n\nenum ast_symbols {");
  }

  /*
  .. DFS, Post Order.
  .. Each instance, you go back up the chain, 
  .. you can reduce properties, Ex : 
  ..    f(parent) = max { f(child) | each 'child' of 'parent' }
  */

  while (level >= 0) {
    /* Go down till the max depth. */
    while ( push (&level) ) {};

    /* Do something with rule*/ 
    _Rule * rule = stack[level].rule;
    (void) rule;
    if ( !traversal_number )  
      STRING_APPEND(pool,"\n  sym_%s", rule->name);

    /* Pop */
    pop( &level );
  }

  if ( !traversal_number ) { 
    STRING_APPEND(pool,"\n}");
    fprintf(stderr, "%s", symbols);
  }

  traversal_number++;
}
