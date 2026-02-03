  /**
  ..  TODO: 
  replace _Flag with int
  replace _Name to name
  replace NameFunc to name_func
  remove headers which are non C99 std
  reduce number of stdarg usages
  check mem alloc/dealloc of attributes
  tab before each line in between #ifndef #endif
  c extern
  */
  
#ifndef _HEDGE_MESH_COMMON
#define _HEDGE_MESH_COMMON
#ifdef __cplusplus
  extern "C" {
#endif
  
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
  .. =================== Dynamic Array ========================================
  .. _Array is  used to store dynamic data. You can create, reallocate, shrink
  .. and free the "Array" without any memory mismanagement. NOTE: realloc()
  .. functn used inside array_append(), can slow the program, when you append
  .. large data multiple times. WHY? Because realloc search for continous chunk
  .. of memory that can fit the new array data which might be very large. 
  */
  typedef struct {
    void * p;
    size_t max, len;
  } _Array;
  
  /* 
  .. Common API related to _Array.
  .. (a) Create and return a new array
  .. (b) Free the array 'a'
  .. (c) Copy 'size' bytes of 'data' to the array 'a' 
  .. (d) Shrink 'a->p' to 'a->len' bytes
  */
  extern _Array * array_new();
  extern void     array_free   (_Array * a);
  extern void     array_append (_Array * a, void * data, size_t size);
  extern void     array_shrink (_Array * a);
  
  
  /*
  .. ====================  Error Handling  ====================================
  .. Generally used rule, (not strict)
  .. (a) return non-zero for error
  .. (b) For pointer type return, NULL return maybe an error
  .. Remember this is not strictly followed.
  */
  enum HMESH_ERROR_TYPES {
    HMESH_NO_ERROR = 0,
    HMESH_ERROR = 1
  };
  
  /*
  .. Common API for error handling
  .. (a) Append the error string 'err' to the buffer.
  .. (b) Flush the error
  .. (c) Send the starting memory index of error string
  */
  extern void   hmesh_error       (const char *format, ...);
  extern void   hmesh_error_flush (int fd);
  extern char * hmesh_error_get   ();
    
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
    _Index  in_use,
            free_list,
            loc,
            loc_free;
  } _IndexInfo ; 
  
  typedef struct {
    /* indices [0,max) are either in use/free . Index UINT16_MAX 
    .. is reserved and used in place of NULL.
    .. This is only for small 'max' (say max <= 256). 
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
        hmesh_error("IndexStackFreeHead() : Out of index");
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
      hmesh_error("IndexStackDeallocate() : not in use");
      return HMESH_ERROR;
    }
    if( index_att ) {
      hmesh_error("IndexStackDeallocate() : attribute not freed");
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
        hmesh_error("IndexStackFreeHead() : Out of index");
        return HMESH_ERROR;
      }
    _IndexInfo * info = stack->info;
    /* index location in free_list array */
    _Index indexLoc   = info[index].loc_free;
    void *** att = stack->attribute;
    void * index_att = att ? (*att)[index] : NULL;
    if( indexLoc == UINT16_MAX || index_att ) {
      hmesh_error("IndexStackAllocate() : "
                 "index %d already in use", index);
      return HMESH_ERROR;
    }
  
    info[stack->n].in_use    = index;
    info[index].loc          = stack->n++;
    info[indexLoc].free_list = info[--(stack->nfree)].free_list;
    info[info[indexLoc].free_list].loc_free = indexLoc;
    info[index].loc_free     = UINT16_MAX;
  
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
        hmesh_error ("IndexStackDestroy() : "
                     "index %d cannot be freed", index);
        status = HMESH_ERROR;
      }
    }
    if(status == HMESH_ERROR)
      return status;
  
    if(stack->info)
      free(stack->info);
    stack->info = NULL;
  
    return HMESH_NO_ERROR;
  }
  
#ifdef __cplusplus
  }
#endif
#endif
