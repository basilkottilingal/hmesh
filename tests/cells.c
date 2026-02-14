#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  HmeshCells * vertices = hmesh_cells (0,3);

  hmesh_scalar_new (vertices, "s");
  hmesh_scalar_new (vertices, "0"); //Error : naming should follow C naming rules
  hmesh_scalar_new (vertices, "v");
  hmesh_scalar_new (vertices, "s"); //Error : 's' already exits
  hmesh_scalar_new (vertices, "veryLongScalarNameIsNotAllowedFoScalarName"); //Error : long name

  hmesh_error_flush ();

  hmesh_scalar_remove (vertices, "s");
  hmesh_scalar_new (vertices, "s");
  hmesh_scalar_remove (vertices, "z"); //error : 'z' doesn't exist

  hmesh_error_flush ();

  fprintf(stdout, "\nList of attr");  
  for (int i=0; i<vertices->min; ++i)
  {
    HmeshArray * attr = (HmeshArray *) vertices->attr[i];
    assert (attr);
    fprintf (stdout, "\n\t%s", attr->name);
  }
  fprintf (stdout, "\nList of scalar attr");  
  for (int i = vertices->min; i<vertices->scalars.n; ++i)
  {
    Index iscalar = vertices->scalars.info[i].in_use;
    HmeshArray * attr = (HmeshArray *) vertices->attr[iscalar];
    assert (attr);
    fprintf (stdout, "\n\t%s", attr->name);
  }

  hmesh_cells_destroy (vertices);

  hmesh_tpool_destroy ();

  hmesh_error_flush ();

  return 0;
}
