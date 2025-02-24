/**TODO:
*/

#ifndef _HMESH_COMMON
#define _HMESH_COMMON

#include <stdlib.h>
#include <stdio.h>

/* '_Flag' : used for unsigned numbers in the range [0,256), 
..  (a) counting numbers < 256, 
..  (b) error flags, 
..  etc.
*/
typedef uint8_t _Flag;

/* ---------------------------------------------------------
------------------------------------------------------------
  An "Array" is used to store dynamic data. You can create, 
  .. reallocate, shrink and free the "Array" ..
  .. without any memory mismanagement. 
  .. NOTE: realloc() function used inside array_append(),
  .. can slow the program, when you append large data ..
  .. multiple times. WHY? : Because realloc search for .. 
  .. continous chunk of memory that can fit the new ..
  .. array data which might be very large. In this case, ..
  .. impelement memory pooling of fixed sized blocks ..
  .. Refer to:
      http://www.boost.org/doc/libs/1_55_0/libs/pool/doc/html/boost_pool/pool/pooling.html
------------------------------------------------------------
--------------------------------------------------------- */
typedef struct {
  void * p;
  size_t max, len;
} _Array;

_Array * ArrayNew()
{
  _Array * a = (_Array *) malloc (sizeof(_Array));
  a->p = NULL;
  a->max = a->len = 0;
  return a;
}

void ArrayFree (_Array * a)
{
  free (a->p);
  free (a);
}

void ArrayAppend (_Array * a, void * data, size_t size)
{
  if (a->len + size >= a->max) {
    a->max += size >4096 ? size : 4096;
    a->p = realloc (a->p, a->max);
  }
  memcpy (((char *)a->p) + a->len, data, size);
  a->len += size;
}

void ArrayShrink (_Array * a)
{
  if(a->len < a->max) {
    a->p = realloc (a->p, a->len);
    a->max = a->len;
  }
}

/*
Error Handling
*/

_Array HmeshErrorBuffer = {.p = NULL,.len = 0,.max = 0};

extern
void HmeshError(char * err);

extern
void HmeshErrorPrint();
#endif
