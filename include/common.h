#ifndef _HMESH_COMMON_
#define _HMESH_COMMON_

#ifdef __cplusplus
extern "C" {
#endif
  
  #include <stdio.h>
  #include <assert.h>
  #include <string.h>
  #include <stdarg.h>
  #include <stdlib.h>
  
  /*
  .. =================== Dynamic Array ========================================
  .. Array is  used to store dynamic data. You can create, reallocate, shrink
  .. and free the "Array" without any memory mismanagement. NOTE: realloc()
  .. functn used inside array_append(), can slow the program, when you append
  .. large data multiple times. WHY? Because realloc search for contigous chunk
  .. of memory that can fit the new array data which might be very large. 
  */
  typedef struct {
    void * p;
    size_t max, len;
  } Array;
  
  /* 
  .. Common API related to Array.
  .. (a) Create and return a new array
  .. (b) Free the array 'a'
  .. (c) Copy 'size' bytes of 'data' to the array 'a' 
  .. (d) Shrink 'a->p' to 'a->len' bytes
  */
  extern Array * array_new    ( );
  extern void    array_free   (Array * a);
  extern void    array_append (Array * a, void * data, size_t size);
  extern void    array_shrink (Array * a);
  
  /*
  .. ====================  Error Handling  ====================================
  .. Generally used rule, (not strict)
  .. (a) return non-zero for error
  .. (b) For pointer type return, NULL return maybe an error
  .. Remember this is not strictly followed.
  */
  enum HMESH_ERROR_TYPES {
    HMESH_NO_ERROR = 0,                   /* Success flag                    */
    HMESH_ERROR    = 1,                   /* unknown/non-specific error flag */
    HMESH_ERROR_OM = 2,                   /* Error : out of memory           */
    HMESH_ERROR_NI = 3,                   /* error : not yet implemented     */
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
    
  /* 
  .. Real : Precision used in scientific computation.
  .. 32 bits is Preferred data type precision for GPU vectors.
  .. Everywhere else (and by default) it's used as 64.
  */
  
  #if HMESH_PRECISION == 32
  typedef float  Real;
  #else 
  typedef double Real;
  #endif
  
  /*
  .. short int, used for indices of node arrays indices [0,max) are either in
  .. use/free . Index UINT16_MAX is reserved and used in place of NULL.This is
  .. only for small 'max' (say max <= 256). For each 'index' in in_use, you may
  .. store attributes in (*attributes)[index]
  */

  typedef uint16_t Index;

  typedef struct
  {
    Index  in_use,
           free_list,
           loc,
           loc_free;
  } IndexInfo ;

  typedef struct
  {
    Index  increment,
           limit,
           n,
           nfree,
           max;
    IndexInfo * info;
    void ***    attribute;
  } IndexStack;
  
  /*
  .. Functions related to IndexStack
  */
  
  static inline IndexStack 
  index_stack (Index limit, Index increment, void *** att)
  {
  
    assert(limit > 1);
    increment = increment ? increment : 1;
    Index max = increment > limit ? limit : increment;
    IndexInfo  * info = 
        (IndexInfo *) malloc ( max* sizeof(IndexInfo)); 
  
    if(att) { 
      assert(*att == NULL);
      *att = (void **) malloc (max * sizeof(void *));
    }
    for(Index i=0; i<max; ++i) {
      info[i].free_list = i;
      info[i].loc_free  = i;
      info[i].loc       = UINT16_MAX;
      if(att)
        (*att)[i] = NULL;
    }
    return (IndexStack) {
      .info = info,
      .increment = increment,
      .limit = limit, 
      .n = 0,
      .nfree = max,
      .max = max,
      .attribute = att
    };
  }
  
  static inline Index 
  index_stack_expand (IndexStack * stack)
  {
    if(stack->max == stack->limit)  
      /* Reached limit. Return error*/
      return HMESH_ERROR;
    /* Expand the array */
    Index max = stack->max + stack->increment; 
    max = (max > stack->limit) ? stack->limit : max;
    stack->info = (IndexInfo *) 
      realloc (stack->info, max * sizeof(IndexInfo));
      
    void *** att = stack->attribute;
    if(att)
      *att = (void **) realloc(*att, max * sizeof(void *));
  
    IndexInfo * info = stack->info;
    for(Index i = stack->max; i<max; ++i) {
      info[stack->nfree].free_list = i;
      info[i].loc_free             = stack->nfree++;
      info[i].loc                  = UINT16_MAX;
      if(att)
        (*att)[i] = NULL;
    }
    stack->max = max;
  
    return HMESH_NO_ERROR;
  }
  
  static inline Index 
  index_stack_free_head (IndexStack * stack, int isPop)
  {
    if (!stack->nfree) 
      if (index_stack_expand(stack) == HMESH_ERROR) {
        hmesh_error("index_stack_free_head() : Out of index");
        /* Reached limit. Return invalid index*/
        return UINT16_MAX;
      }
    IndexInfo * info = stack->info;
    Index index = info[stack->nfree-1].free_list;
    if(isPop) { 
      --(stack->nfree);
      info[index].loc_free  = UINT16_MAX;
      info[stack->n].in_use = index;
      info[index].loc       = stack->n++;
    }
    return index;
  }
  
  static inline int 
  index_stack_deallocate (IndexStack * stack, Index index)
  {
    /* index location in in_use array */
    Index indexLoc = (index >= stack->max) ? UINT16_MAX : 
                          stack->info[index].loc;
    void *** att = stack->attribute;
    void * index_att = att ? (*att)[index] : NULL;
    if( indexLoc == UINT16_MAX ) {
      hmesh_error("index_stack_deallocate() : not in use");
      return HMESH_ERROR;
    }
    if( index_att ) {
      hmesh_error("index_stack_deallocate() : attribute not freed");
      return HMESH_ERROR;
    }
    IndexInfo * info             = stack->info;
    info[stack->nfree].free_list = index;
    info[index].loc_free         = stack->nfree++;
    info[indexLoc].in_use        = info[--(stack->n)].in_use;
    info[info[indexLoc].in_use].loc = indexLoc;
    info[index].loc              = UINT16_MAX;
  
    return HMESH_NO_ERROR;
  }
  
  static inline int 
  index_stack_allocate (IndexStack * stack, Index index)
  {
    while ( index >= stack->max ) 
      if ( index_stack_expand(stack) == HMESH_ERROR ) {
        hmesh_error("index_stack_free_head() : Out of index");
        return HMESH_ERROR;
      }
    IndexInfo * info = stack->info;
    /* index location in free_list array */
    Index indexLoc   = info[index].loc_free;
    void *** att = stack->attribute;
    void * index_att = att ? (*att)[index] : NULL;
    if ( indexLoc == UINT16_MAX || index_att ) {
      hmesh_error("index_stack_allocate() : "
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
  
  static inline int
  index_stack_destroy (IndexStack * stack)
  {
    Index n = stack->n;
    int status = HMESH_NO_ERROR;
    while (n--) {
      Index index = stack->info[n].in_use;
      if ( index_stack_reallocate (stack, index) == HMESH_ERROR ) { 
        /* Attributes are not freed */
        hmesh_error ("index_stack_destroy() : "
                     "index %d cannot be freed", index);
        status = HMESH_ERROR;
      }
    }
    if (status == HMESH_ERROR)
      return status;
  
    if (stack->info)
      free(stack->info);
    stack->info = NULL;
  
    return HMESH_NO_ERROR;
  }
  
#ifdef __cplusplus
}
#endif
#endif
