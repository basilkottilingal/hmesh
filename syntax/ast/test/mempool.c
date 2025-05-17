// gcc -o test mempool.c ../memory.o
// ./test

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../memory.h"

  
typedef struct  {
  char c[128];
} str;

int main() {

  /* 8 Bytes taken from pool handler */
  double * d1 = (double *) ast_allocate_internal(sizeof(double));
  char * end = (char *) d1 + 8;
  fprintf(stdout, "\nMemory Upper bound (blocks[0]) %p\n", end); 

  /* 500 * sizeof(str *) = 4000 B, taken from pool handler*/
  str ** ptr = (str **) 
    ast_allocate_internal(500 * sizeof(str *));

  /* create a pool for str.
  .. which by default consumes a page (4096 B) from pool handler*/
  _AstPool * pool = ast_pool(sizeof(str));


  char sample[] = "my name is algorithm. If I am written carefully"
    " and after a very long thought process you can create magic."
    " A small mistake may create havoc. ";
  srand(time(0));
  size_t len = strlen(sample);
  
  for(size_t i=0; i<4096/sizeof(str); ++i) {
    ptr[i] = (str *) ast_allocate_from(pool);
    char * s = (char *) sample;
    size_t lc = rand() % len;
    s += lc;
    strncpy( ptr[i]->c, s, 127);
  }
 
  double * d2 = (double *) ast_allocate_internal(sizeof(double));
  fprintf(stdout, "Compare %zd %zd (Is it same ?)", 
    (char *) d1 - ((char *) d2 + sizeof(double)),
    500*sizeof(str *) + 4096 );

  /*
  .. Let's see if pool handler can handle more than 1 MB
  */
  size_t n = (1<<20) / sizeof(str) + 10;
  while(n--) {
    str * s = (str *) ast_allocate_from (pool);
  }

  /*
  .. Print 
  */
  for(size_t i=0; i<4096/sizeof(str); ++i) {
    fprintf(stdout, "\n : %s", ptr[i]->c);
  }

  /*
  .. free all
  */   
  ast_deallocate_all();
}
