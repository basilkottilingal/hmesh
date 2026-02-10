#include <common.h>
#include <hmesh.h>

int main()
{

  void ** mem1, ** mem2, ** mem3;
  mem1 = mem2 = mem3 = NULL;

  HmeshArray * att = hmesh_array ("s", 8, &mem1);
  Index iblock = 3;
  while (iblock--)
  {
    if( !hmesh_array_add ( att, iblock, &mem1 ))
    {
      hmesh_error ("main() : cannot create "
        "iblock %d in attr \"%s\"", iblock, att->name);
      continue;
    }

    double * val = (double *) mem1[iblock];
    for (Index i=0; i<hmesh_tpool_block_size(); ++i, ++val)
      *val = 0.011;
  }

  iblock = 5;
  while (iblock--)
  {
    /* NOTE iblock 4,3 should give error */
    if( hmesh_array_remove (att, iblock, &mem1) )
    {
      hmesh_error ("main() : cannot remove "
        "iblock %d in attr \"%s\"", iblock, att->name);
    }
  }

  if( hmesh_array_destroy (att, &mem1) )
  {
      hmesh_error ("main() : att remove err");
  }
  hmesh_error_flush ();

  /* warning for no name */
  HmeshArray * att1 = hmesh_array (NULL, 8, &mem2);
  HmeshArray * att2 = hmesh_array ("", 8, &mem3);
  hmesh_array_destroy (att1, &mem2);
  hmesh_array_destroy (att2, &mem3);
  hmesh_error_flush ();


  char aname[200] = "this_is_a_very_long_attribute_name_"
    "which_can_be_potentially_trunked_without_warning_and_can_cause_problem";
  mem2 = NULL;
  att = hmesh_array (aname, 1, &mem2);
  if (strcmp (aname, att->name))
    hmesh_error ("atttribute name trunked to '%s'", att->name);
  hmesh_array_destroy (att, &mem2);
  hmesh_error_flush ();

  return 0;
}
