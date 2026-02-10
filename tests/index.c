#include <common.h>

int main()
{
  hmesh_error ("error checking");
  hmesh_error ("Checking");
  hmesh_error_flush ();

  IndexStack stack = index_stack (10, 3, NULL);  

  for (Index i = 0; i < 15; ++i)
  {
    if (index_stack_free_head (&stack, 1) == UINT16_MAX)
      hmesh_error ("Out of free index");
  }
  /* It should print the error 5 times */
  hmesh_error_flush ();

  for (Index index = 5; index < 20; ++index)
  {
    if (index_stack_deallocate (&stack, index) == HMESH_ERROR)
      hmesh_error ("index %d not in use", index);
  }
  /* indices 10-14 are not in use */
  hmesh_error_flush ();

  for (Index index = 0; index < 10; ++index)
    if (index_stack_allocate (&stack, index) == HMESH_ERROR)
      hmesh_error ("index %d still in use", index);
  /* indices 0-4 are still in use */
  hmesh_error_flush ();

  /* Delete arrays */
  index_stack_destroy (&stack);

  /* Create a new stack with attribute */
  void ** attributes = NULL;
  stack = index_stack (10, 6, &attributes);

  for (Index i = 0; i < 10; ++i)
  {
    Index index = index_stack_free_head (&stack, 1);
    if (index != UINT16_MAX) 
      attributes[index] = malloc(8);
  }

  for (Index index = 0; index < 10; ++index)
  {
    if (index&1)
    {
      free(attributes[index]);
      attributes[index] = NULL;
    }
    if (index_stack_deallocate (&stack, index) == HMESH_ERROR)
      hmesh_error("attribute of index %d still not freed", index);
  }
  /* even number indices shows error */
  hmesh_error_flush ();

  /* Delete arrays */
  if (index_stack_destroy (&stack) == HMESH_ERROR)
    hmesh_error ("attribute of index(indices) still not freed");
  /* expect error */
  hmesh_error_flush ();

  hmesh_error ("There should be no error!");
  for (Index index = 0; index < 10; ++index)
  {
    if (attributes[index])
    {
      free (attributes[index]);
      attributes[index] = NULL;
    }
  }
  if (index_stack_destroy (&stack) == HMESH_ERROR)
    hmesh_error ("attribute of index(indices) still not freed");
  /* expect NO error */
  hmesh_error_flush ();

  return 0;
}
