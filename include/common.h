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
  _Index in_use, free_list, loc, loc_free;
} _IndexInfo ; 

typedef struct {
  /* indices [1,max) are either in use/free . Index UINT16_MAX 
  .. is reserved and used in place of NULL.
  .. This is only for small 'max' (say <= 255). 
  */
  _IndexInfo * info;
  _Index increment, limit, n, nfree, max;

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
  _Index max = increment > limit ? limit : increment;
  _IndexInfo  * info = 
      (_IndexInfo *) malloc ( max* sizeof(_IndexInfo)); 

  if(att) { 
    assert(*att == NULL);
    *att = (void **) malloc (max * sizeof(void *));
  }
  for(_Index i=0; i<max; ++i) {
    info[i].free_list = i;
    info[i].loc_free  = i;
    info[i].loc       = UINT16_MAX;
    if(att)
      (*att)[i] = NULL;
  }
  return (_IndexStack) 
    {.info = info, .increment = increment, .limit = limit, 
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
  stack->info = (_IndexInfo *) 
    realloc (stack->info, max * sizeof(_IndexInfo));
    
  void *** att = stack->attribute;
  if(att)
    *att = (void **) realloc(*att, max * sizeof(void *));

  _IndexInfo * info = stack->info;
  for(_Index i = stack->max; i<max; ++i) {
    info[stack->nfree].free_list = i;
    info[i].loc_free             = stack->nfree++;
    info[i].loc                  = UINT16_MAX;
    if(att)
      (*att)[i] = NULL;
  }
  stack->max = max;

  return HMESH_NO_ERROR;
}

static inline _Index 
IndexStackFreeHead(_IndexStack * stack, int isPop) {
  if(!stack->nfree) 
    if(IndexStackExpand(stack) == HMESH_ERROR) {
      HmeshError("IndexStackFreeHead() : Out of index");
      /* Reached limit. Return invalid index*/
      return UINT16_MAX;
    }
  _IndexInfo * info = stack->info;
  _Index index = info[stack->nfree-1].free_list;
  if(isPop) { 
    --(stack->nfree);
    info[index].loc_free  = UINT16_MAX;
    info[stack->n].in_use = index;
    info[index].loc       = stack->n++;
  }
  return index;
}

static inline _Flag 
IndexStackDeallocate(_IndexStack * stack, _Index index) {
  /* index location in in_use array */
  _Index indexLoc = (index >= stack->max) ? UINT16_MAX : 
                        stack->info[index].loc;
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
  _IndexInfo * info = stack->info;
  info[stack->nfree].free_list = index;
  info[index].loc_free         = stack->nfree++;
  info[indexLoc].in_use        = info[--(stack->n)].in_use;
  info[info[indexLoc].in_use].loc = indexLoc;
  info[index].loc              = UINT16_MAX;

  return HMESH_NO_ERROR;
}

static inline _Flag 
IndexStackAllocate(_IndexStack * stack, _Index index) {
  while( index >= stack->max ) 
    if( IndexStackExpand(stack) == HMESH_ERROR ) {
      HmeshError("IndexStackFreeHead() : Out of index");
      return HMESH_ERROR;
    }
  _IndexInfo * info = stack->info;
  /* index location in free_list array */
  _Index indexLoc   = info[index].loc_free;
  void *** att = stack->attribute;
  void * index_att = att ? (*att)[index] : NULL;
  if( indexLoc == UINT16_MAX || index_att ) {
    HmeshError("IndexStackAllocate() : "
               "index %d already in use", index);
    return HMESH_ERROR;
  }

  info[stack->n].in_use     = index;
  info[index].loc           = stack->n++;
  info[indexLoc].free_list  = info[--(stack->nfree)].free_list;
  info[info[indexLoc].free_list].loc_free = indexLoc;
  info[index].loc_free      = UINT16_MAX;

  return HMESH_NO_ERROR;
}

static inline _Flag
IndexStackDestroy(_IndexStack * stack) {
  _Index n = stack->n;
  _Flag status = HMESH_NO_ERROR;
  while(n--) {
    _Index index = stack->info[n].in_use;
    if( IndexStackDeallocate(stack, index) == HMESH_ERROR ) { 
      /* Attributes are not freed */
      HmeshError("IndexStackDestroy() : "
                 "index %d cannot be freed", index);
      status = HMESH_ERROR;
    }
  }
  if(status == HMESH_ERROR)
    return status;

  free(stack->info);

  return HMESH_NO_ERROR;
}

#endif
