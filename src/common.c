#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <common.h>

/*
.. Function definitions related to _Array
*/

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
.. definition related to error handling
*/

char * HmeshErrorGet() {
  return (char *) _HMESH_ERROR_BUFFER_.p;  
}

void HmeshError(char * err) {
  _HMESH_ERROR_ |= _HMESH_ERROR;
  _Array * b = &_HMESH_ERROR_BUFFER_;  
  _Flag n = 0;
  char * c = err, end[2] = {'\n','\0'};
  /* Get the count of char in err*/
  while(n<UINT8_MAX-1 && *c++)
    n++;
  if(!n)
    return;
  if(b->len)
    b->len--;
  else {
    char e[] = "\n=======Error=======\n";
    ArrayAppend(b, e, 21);
  }
  /* Copy error to buffer. Excluding the '\0' */
  ArrayAppend(b, err, n);
  ArrayAppend(b, end, 2);
}

static
void HmeshErrorFree() {
  /* Empty all error message. 
  .. Set as HMESH_NO_ERROR.
  */
  _HMESH_ERROR_ = _HMESH_NO_ERROR;
  _Array * b = &_HMESH_ERROR_BUFFER_;  
  b->len = b->max = 0;
  if(b->p)
    free(b->p);
  b->p = NULL;
}

void HmeshErrorFlush(int fd) {
  _Array * b = &_HMESH_ERROR_BUFFER_;  
  if(!b->p)
    return;
  fd = fd < 2 ? 2 : fd;
  if ( fcntl(fd, F_GETFD) == -1 || errno == EBADF ) {
    fprintf(stderr, "Error : 'fd' not available for output");
    fflush(stderr);
    return;
  }
  if( !write(fd, (char *) b->p, b->len) ) {
    fprintf(stderr, 
      "Error : Couldn't write msg to 'fd'");
    fflush(stderr);
    return;
  }
  HmeshErrorFree(); 
}
