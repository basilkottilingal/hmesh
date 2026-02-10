#include <common.h>

Array * array_new ()
{
  Array * a = (Array *) malloc (sizeof (Array));
  a->p = NULL;
  a->max = a->len = 0;
  return a;
}

void array_free (Array * a)
{
  free (a->p);
  free (a);
}

void array_append (Array * a, void * data, size_t size)
{
  if (a->len + size >= a->max)
  {
    a->max += size >4096 ? size : 4096;
    a->p = realloc (a->p, a->max);
  }
  memcpy (((char *)a->p) + a->len, data, size);
  a->len += size;
}

void array_shrink (Array * a)
{
  if (a->len < a->max)
  {
    a->p = realloc (a->p, a->len);
    a->max = a->len;
  }
}

/*
.. Definition related to error handling. Error is stored as char array in
.. 'HMESH_ERROR_MSG_BUFFER'
*/
static int   HMESH_ERROR_STATUS     = 0;
static Array HMESH_ERROR_MSG_BUFFER = { .p = NULL, .len = 0, .max = 0 };

/*
.. Append error msg. Warning: error msg will be truncated to 100 chars
*/
void hmesh_error (const char * err, ...)
{
  char errmsg[100];
  va_list args;
  va_start (args, err);
  vsnprintf (errmsg, 100, err, args);
  va_end (args);

  HMESH_ERROR_STATUS |= HMESH_ERROR;
  Array * b = &HMESH_ERROR_MSG_BUFFER;
  char end[] = "\n";
  if (b->len)
    b->len--;
  else
  {
    char e[] = "\n=======Error=======\n";
    array_append (b, e, 21);
  }
  array_append (b, errmsg, strlen (errmsg));
  array_append (b, end, 2);
}

static
void hmesh_error_free ()
{
  HMESH_ERROR_STATUS = HMESH_NO_ERROR;
  Array * b = &HMESH_ERROR_MSG_BUFFER;
  b->len = b->max = 0;
  if (b->p)
    free (b->p);
  b->p = NULL;
}

void hmesh_error_flush ( )
{
  char * msg = (char *) HMESH_ERROR_MSG_BUFFER.p;
  if (!msg)
    return;
  fprintf (stderr, "%s", msg);
  hmesh_error_free ();
}
