/**
..  TODO:
*/

#ifndef _HEDGE_MESH_COMMON
#define _HEDGE_MESH_COMMON

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdint.h>

/* 
.. '_Flag' : used for unsigned numbers in the range [0,256), 
..  (a) counting numbers < 256, 
..  (b) error flags, 
..  etc.
*/
typedef uint8_t _Flag;
#define HMESH_FLAG_MAX UINT8_MAX

/* 
.. An "Array" is used to store dynamic data. You can create, 
.. reallocate, shrink and free the "Array" ..
.. without any memory mismanagement. 
.. NOTE: realloc() function used inside array_append(),
.. can slow the program, when you append large data ..
.. multiple times. WHY? : Because realloc search for .. 
.. continous chunk of memory that can fit the new ..
.. array data which might be very large. 
*/
typedef struct {
  void * p;
  size_t max, len;
} _Array;

/* 
.. Create and return a new array
*/
extern _Array * ArrayNew();

/* 
.. Free the array 'a'
*/
extern void ArrayFree(_Array * a);

/* 
.. Copy 'size' bytes of 'data' to the array 'a' 
*/
extern void ArrayAppend(_Array * a, void * data, size_t size);

/* 
.. Shrink 'a->p' to 'a->len' bytes
*/
extern void ArrayShrink(_Array * a);


/*
.. Error Handling general for the entire hedge mesh project.
*/

enum HMESH_ERROR_TYPES {
  HMESH_NO_ERROR = 0,
  HMESH_ERROR = 1
};

/*
.. Append the error string 'err' to the buffer.
*/
extern void HmeshError(const char *format, ...);

/* 
.. Flush the error buffer to stream with file descriptor 'fd'.
.. In case fd < 2, it will be flushed to stderr.
*/
extern void HmeshErrorFlush(int fd);

/*
.. Send the starting memory index of error string
*/
extern char * HmeshErrorGet();
  
/* _Real : Precision used in scientific computation.
.. 32 bits is Preferred data type precision for GPU vectors.
.. Everywhere else (and by default) it's used as 64.
*/

#ifdef _PRECISION32
typedef float _Real;
#else 
typedef double _Real;
#endif

/* short int, used for indices of node arrays */  
typedef uint16_t _Index;

typedef struct {
  /* indices [1,max) are either in use/free . Index UINT16_MAX 
  .. is reserved and used in place of NULL.
  .. This is only for small 'max' (say <= 255). 
  */
  _Index * in_use, * free_list, * loc,  * loc_free,
           increment, limit, n, nfree, max;

  /* for each 'index' in in_use, you may store attributes
  .. in (*attributes)[index] */
  void *** attribute;
} _IndexStack;

/*
.. Functions related to _IndexStack
*/

static inline _IndexStack 
IndexStack(_Index limit, _Index increment, void *** att) {
  assert(limit > 1);
  increment = increment ? increment : 1;
  _Index max = increment > limit ? limit : increment,
    * in_use = (_Index *) malloc (max* sizeof(_Index)), 
    * free_list = (_Index *) malloc (max* sizeof(_Index)),
    * loc = (_Index *) malloc (max* sizeof(_Index)),
    * loc_free = (_Index *) malloc (max* sizeof(_Index));
  if(att) { 
    assert(*att == NULL);
    *att = (void **) malloc (max * sizeof(void *));
  }
  for(_Index i=0; i<max; ++i) {
    free_list[i] = i;
    loc_free[i] = i;
    loc[i] = UINT16_MAX;
    if(att)
      (*att)[i] = NULL;
  }
  return (_IndexStack) 
    {.in_use = in_use, .free_list = free_list, 
     .loc = loc, .loc_free = loc_free, 
     .increment = increment, .limit = limit, 
     .n = 0, .nfree = max, .max = max, .attribute = att} ;
}

static inline _Index 
IndexStackExpand(_IndexStack * stack) {
  if(stack->max == stack->limit)  
    /* Reached limit. Return error*/
    return HMESH_ERROR;
  /* Expand the array */
  _Index max = stack->max + stack->increment; 
  max = (max > stack->limit) ? stack->limit : max;
  stack->in_use = (_Index *) 
    realloc (stack->in_use, max * sizeof(_Index));
  stack->free_list = (_Index *) 
    realloc (stack->free_list, max * sizeof(_Index));
  stack->loc = (_Index *) 
    realloc (stack->loc, max * sizeof(_Index));
  stack->loc_free = (_Index *)
    realloc (stack->loc_free, max * sizeof(_Index));
    
  void *** att = stack->attribute;
  if(att)
    *att = (void **) realloc(*att, max * sizeof(void *));

  for(_Index i = stack->max; i<max; ++i) {
    stack->free_list[stack->nfree] = i;
    stack->loc_free[i] = stack->nfree++;
    stack->loc[i] = UINT16_MAX;
    if(att)
      (*att)[i] = NULL;
  }
  stack->max = max;

  return HMESH_NO_ERROR;
}

static inline _Index 
IndexStackFreeHead(_IndexStack * stack, int isPop) {
  if(!stack->nfree) 
    if(IndexStackExpand(stack) == HMESH_ERROR)
      /* Reached limit. Return invalid index*/
      return UINT16_MAX;
  _Index index = stack->free_list[stack->nfree-1];
  if(isPop) { 
    --(stack->nfree);
    stack->loc_free[index] = UINT16_MAX;
    stack->in_use[stack->n] =  index;
    stack->loc[index] = stack->n++;
  }
  return index;
}

static inline _Flag 
IndexStackDeallocate(_IndexStack * stack, _Index index) {
  /* index location in in_use array */
  _Index indexLoc = 
    (index >= stack->max) ? UINT16_MAX : stack->loc[index];
  void *** att = stack->attribute;
  void * index_att = att ? (*att)[index] : NULL;
  if( indexLoc == UINT16_MAX ) {
    HmeshError("IndexStackDeallocate() : not in use");
    return HMESH_ERROR;
  }
  if( index_att ) {
    HmeshError("IndexStackDeallocate() : attribute not freed");
    return HMESH_ERROR;
  }
  stack->free_list[stack->nfree] = index;
  stack->loc_free[index] = stack->nfree++; 
  stack->in_use[indexLoc] = stack->in_use[--(stack->n)];
  stack->loc[stack->in_use[indexLoc]] = indexLoc;
  stack->loc[index] =  UINT16_MAX;

  return HMESH_NO_ERROR;
}

static inline _Flag 
IndexStackAllocate(_IndexStack * stack, _Index index) {
  while( index >= stack->max ) 
    if( IndexStackExpand(stack) == HMESH_ERROR )
      return HMESH_ERROR;
  /* index location in free_list array */
  _Index indexLoc = stack->loc_free[index];
  void *** att = stack->attribute;
  void * index_att = att ? (*att)[index] : NULL;
  if( indexLoc == UINT16_MAX || index_att )
    /* This index is already in use */
    return HMESH_ERROR;
  stack->in_use[stack->n] = index;
  stack->loc[index] = stack->n++; 
  stack->free_list[indexLoc] = stack->free_list[--(stack->n)];
  stack->loc_free[stack->free_list[indexLoc]] = indexLoc;
  stack->loc_free[index] =  UINT16_MAX;

  return HMESH_NO_ERROR;
}

static inline _Flag
IndexStackDestroy(_IndexStack * stack) {
  _Index n = stack->n;
  _Flag status = HMESH_NO_ERROR;
  while(n--) {
    _Index index = stack->in_use[n];
    if( IndexStackDeallocate(stack, index) == HMESH_ERROR ) { 
      /* Attributes are not freed */
      HmeshError("IndexStackDestroy() : "
                 "index %d cannot be freed", index);
      status = HMESH_ERROR;
    }
  }
  if(status == HMESH_ERROR)
    return status;

  free(stack->free_list);
  free(stack->in_use);
  free(stack->loc);
  free(stack->loc_free);

  return HMESH_NO_ERROR;
}

#endif
