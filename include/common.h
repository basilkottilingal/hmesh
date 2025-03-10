/**
..  TODO:
*/

#ifndef _HEDGE_MESH_COMMON
#define _HEDGE_MESH_COMMON

#include <stdio.h>
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

#endif
