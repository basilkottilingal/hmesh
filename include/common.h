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
  #include <stdint.h>

  /*
  .. =================== Dynamic Array ========================================
  .. Array is  used to store dynamic data. You can create, reallocate, shrink
  .. and free the "Array" without any memory mismanagement. NOTE: realloc ()
  .. functn used inside array_append (), can slow the program, when you append
  .. large data multiple times. WHY? Because realloc search for contigous chunk
  .. of memory that can fit the new array data which might be very large.
  */
  typedef struct
  {
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
  enum HMESH_ERROR_TYPES
  {
    HMESH_NO_ERROR = 0,                   /* Success flag                    */
    HMESH_ERROR    = 1,                   /* unknown/non-specific error flag */
    HMESH_ERROR_OM = 2,                   /* Error : out of memory           */
    HMESH_ERROR_NI = 3,                   /* error : not yet implemented     */
  };

  /*
  .. Common API for error handling
  .. (a) "hmesh_error ()" : Append the error string 'err' to the buffer.
  .. NOTE : (i) verbose level should be implemented. (ii) Compiler fatal error
  .. as "error tree". (iii) varidic function should be replaced by
  .. "void hmesh_error (int err_type);" after compiling all error types.
  .. (b) "hmesh_error_flush ()" : Flush the error to stderr.
  */
  extern void   hmesh_error       (const char *format, ...);
  extern void   hmesh_error_flush ();

  /*
  .. Real : Precision used in scientific computation.
  .. 32 bits is Preferred data type precision for GPU vectors.
  .. Everywhere else (and by default) it's used as 64.
  */

  #if HMESH_PRECprint_index_stackION == 32
  typedef float  Real;
  #else
  typedef double Real;
  #endif

  /*
  .. short int, used for indices of node arrays indices [0, max) are either in
  .. use/free . Index UINT16_MAX is reserved and used in place of NULL.This is
  .. only for small 'max' (say max <= 256). For each 'index' in in_use, you may
  .. store attributes in (*attributes)[index]
  */

  typedef uint16_t Index;

  typedef struct
  {
    Index map, inverse;
  } IndexMap;

  typedef struct
  {
    Index increment, limit;
    Index n, max;
    IndexMap * indirection;
    void *** attribute;
  } IndexStack;

  /*
  .. Functions related to IndexStack
  */

  static inline
  IndexStack index_stack (Index limit, Index increment, void *** att)
  {
    assert (limit > 1);
    increment = increment ? increment : 1;
    Index max = increment > limit ? limit : increment;
    IndexMap * indirection = malloc (max* sizeof (IndexMap));

    for (Index i=0; i<max; ++i)
      indirection[i].map = indirection[i].inverse = i;

    if (att)
    {
      assert (*att == NULL);
      *att = malloc (max * sizeof (void *));
      for (Index i=0; i<max; ++i)
        (*att)[i] = NULL;
    }

    return (IndexStack)
    {
      .indirection = indirection,
      .increment   = increment,
      .limit       = limit,
      .n           = 0,
      .max         = max,
      .attribute   = att
    };
  }

  static inline Index index_stack_expand (IndexStack * stack)
  {
    if (stack->max == stack->limit)
      return HMESH_ERROR;
    Index max = stack->max + stack->increment;
    max = (max > stack->limit) ? stack->limit : max;
    stack->indirection = realloc (stack->indirection, max * sizeof (IndexMap));

    void *** att = stack->attribute;
    IndexMap * indirection = stack->indirection;

    for (Index i = stack->max; i<max; ++i)
      indirection [i].map = indirection [i].inverse = i;

    if (att)
    {
      *att = realloc (*att, max * sizeof (void *));
      for (Index i = stack->max; i<max; ++i)
        (*att)[i] = NULL;
    }

    stack->max = max;

    return HMESH_NO_ERROR;
  }

  #if 0
  #define  print_index_stack \
    fflush (stdout);\
    printf ("\nIndexStack [n%d max%d]", stack->n, stack->max);\
    printf ("\nMap       ");\
    for (int i=0; i<stack->max; ++i)\
      printf ("%2d ", stack->indirection [i].map);\
    printf ("\nInv       ");\
    for (int i=0; i<stack->max; ++i)\
      printf ("%2d ", stack->indirection [i].inverse);\
    printf ("\n          ");\
    for (int i=0; i<stack->n; ++i)\
      printf ("   ");\
    printf ("^"); \
    fflush (stdout);
  #endif

  static inline
  Index index_stack_free_head (IndexStack * stack, int is_pop)
  {
    if (stack->n == stack->max)
      if (index_stack_expand (stack) == HMESH_ERROR)
      {
        hmesh_error ("index_stack_allocate () : Out of limit/memory");
        return UINT16_MAX;
      }

    Index index = stack->indirection [stack->n].map;
    if (is_pop)
      stack->n ++;
    return index;
  }


  static inline
  int index_stack_allocate_at (IndexStack * stack, Index index)
  {
    while (index >= stack->max)
      if (index_stack_expand (stack) == HMESH_ERROR)
      {
        hmesh_error ("index_stack_allocate_at () : Out of index");
        return HMESH_ERROR;
      }

    IndexMap * indirection = stack->indirection;
    Index last = stack->n,
      inv = indirection [index].inverse,
      map = indirection [last].map;

    if (inv < last)
    {
      hmesh_error ("index_stack_allocate_at () : Index in use");
      return HMESH_ERROR;
    }

    indirection [last].map = index;
    indirection [index].inverse = last;

    indirection [inv].map = map;
    indirection [map].inverse = inv;

    stack->n ++;
    return HMESH_NO_ERROR;
  }

  static inline
  int index_stack_deallocate (IndexStack * stack, Index index)
  {
    if (index >= stack->max)
    {
      hmesh_error ("index_stack_deallocate () : invalid index");
      return HMESH_ERROR;
    }

    void *** att = stack->attribute;
    IndexMap * indirection = stack->indirection;
    Index last = stack->n - 1,
      inv = indirection [index].inverse,
      map = indirection [last].map;

    if (inv > last)
    {
      hmesh_error ("index_stack_deallocate () : index not in use");
      return HMESH_ERROR;
    }

    if (att ? (*att)[index] : NULL)
    {
      hmesh_error ("index_stack_deallocate () : attribute not freed");
      return HMESH_ERROR;
    }

    indirection [last].map = index;
    indirection [index].inverse = last;

    indirection [inv].map = map;
    indirection [map].inverse = inv;

    stack->n --;

    return HMESH_NO_ERROR;
  }

  static inline int
  index_stack_destroy (IndexStack * stack)
  {
    Index n = stack->n;
    IndexMap * indirection = stack->indirection;
    if (stack->attribute)
    {
      void *** att = stack->attribute;
      while (n--)
        if ( (*att) [indirection[n].map] )
        {
          hmesh_error ("index_stack_destroy () : index cannot be freed");
          return HMESH_ERROR;
        }
    }

    if (indirection)
      free (indirection);
    stack->n = stack->max = 0;
    stack->indirection = NULL;
    stack->attribute = NULL;
    return HMESH_NO_ERROR;
  }

#ifdef __cplusplus
}
#endif
#endif
