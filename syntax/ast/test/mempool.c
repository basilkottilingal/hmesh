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
/*
.. No malloc is used here.
.. Gives example to create a pool of nodes, allocate a node from pool,
.. allocate strings, allocate directly using ast_allocate_general().
*/
int main() {

  /* 8 Bytes taken from pool handler */
  double * d1 = (double *) ast_allocate_general(sizeof(double));
  char * end = (char *) d1 + 8;
  fprintf(stdout, "\nMemory Upper bound (blocks[0]) %p\n", end); 

  /* 500 * sizeof(str *) = 4000 B, taken from pool handler*/
  str ** ptr = (str **) 
    ast_allocate_general(500 * sizeof(str *));

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
 
  double * d2 = (double *) ast_allocate_general(sizeof(double));
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
  .. String allocator
  */
  char _text[] = "The concept of the algorithm traces back to ancient times, long before the advent of modern computing. The term itself is derived from the name of the 9th-century Persian mathematician Muhammad ibn Musa al-Khwarizmi, whose works introduced systematic methods for solving mathematical problems. His book Al-Kitab al-Mukhtasar fi Hisab al-Jabr wal-Muqabala laid the foundations for algebra and described procedures that resemble modern algorithms. Over time, especially with the rise of mechanical and electronic computing in the 20th century, algorithms evolved into formal step-by-step instructions used to solve problems across mathematics, logic, and computer science. Today, they underpin nearly every aspect of technology — from sorting data to encrypting communication.";
  char * text = _text;
  size_t textlen = strlen(text) - 5;

  char ** string_pool = (char **) ast_allocate_general (30 * sizeof(char*));
  for(int i=0; i<30; i++) {
    char _s[32];
    size_t start = rand () % textlen;
    size_t length = rand() % 24 + 7;
    //strncpy (temp,  (const char *) (text + start), length);
    strncpy (_s, text + start, length);
    _s[length-1] = '\0';
    /* string allocator using ast_strdup() */
    char * new_string = ast_strdup(_s);
    string_pool[i] = new_string;
  }
  fprintf(stdout, "\n\n\n string allocator checking");
  for(int i=0; i<30; i++) {
    fprintf(stdout, "\n :: %s ", string_pool[i]);
  }
  

  /*
  .. free all
  */   
  ast_deallocate_all();
}
