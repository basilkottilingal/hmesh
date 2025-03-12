#include <common.h>

/*
.. Function definitions related to _Array
*/

_Array * ArrayNew() {
  _Array * a = (_Array *) malloc (sizeof(_Array));
  a->p = NULL;
  a->max = a->len = 0;
  return a;
}

void ArrayFree (_Array * a) {
  free (a->p);
  free (a);
}

void ArrayAppend (_Array * a, void * data, size_t size) {
  if (a->len + size >= a->max) {
    a->max += size >4096 ? size : 4096;
    a->p = realloc (a->p, a->max);
  }
  memcpy (((char *)a->p) + a->len, data, size);
  a->len += size;
}

void ArrayShrink (_Array * a) {
  if(a->len < a->max) {
    a->p = realloc (a->p, a->len);
    a->max = a->len;
  }
}

/*
.. definition related to error handling. 
.. Error is stored as char array in 'HMESH_ERROR_MSG_BUFFER_'.
*/

static
_Array HMESH_ERROR_MSG_BUFFER = {.p = NULL,.len = 0,.max = 0};

static 
int HMESH_ERROR_STATUS = 0;

char * HmeshErrorGet() {
  return (char *) HMESH_ERROR_MSG_BUFFER.p;  
}

void HmeshError(const char * err, ...) {
     
  char errmsg[100];
  va_list args;      
  va_start(args, err);
  /* Warning: error msg will be truncated to 100 chars */
  vsnprintf(errmsg, 100, err, args);  
  va_end(args);      

  HMESH_ERROR_STATUS |= HMESH_ERROR;
  _Array * b = &HMESH_ERROR_MSG_BUFFER;  
  char end[] = "\n";
  if(b->len)
    b->len--;
  else {
    char e[] = "\n=======Error=======\n";
    ArrayAppend(b, e, 21);
  }
  /* Copy error to buffer. Excluding the '\0' */
  ArrayAppend(b, errmsg, strlen(errmsg));
  ArrayAppend(b, end, 2);
}

static
void HmeshErrorFree() {
  /* Empty all error message. 
  .. Set as HMESH_NO_ERROR.
  */
  HMESH_ERROR_STATUS = HMESH_NO_ERROR;
  _Array * b = &HMESH_ERROR_MSG_BUFFER;  
  b->len = b->max = 0;
  if(b->p)
    free(b->p);
  b->p = NULL;
}

void HmeshErrorFlush(int fd) {
  _Array * b = &HMESH_ERROR_MSG_BUFFER;  
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

