#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  HmeshCells * edges = hmesh_cells (1,3);

  hmesh_scalar_new (edges, "s");
  hmesh_scalar_new (edges, "0"); //Error : naming should follow C naming rules
  hmesh_scalar_new (edges, "v");
  hmesh_scalar_new (edges, "s"); //Error : 's' already exits
  hmesh_scalar_new (edges, "veryLongScalarNameIsNotAllowedFoScalarName"); //Error : long name

  hmesh_error_flush ();

  hmesh_scalar_remove (edges, "s");
  hmesh_scalar_new (edges, "s");
  hmesh_scalar_remove (edges, "z"); //error : 'z' doesn't exist

  hmesh_error_flush ();

  fprintf (stdout, "\nList of attr");  
  for (int i=0; i<edges->min; ++i)
  {
    HmeshArray * attr = (HmeshArray *) edges->attr[i];
    assert (attr);
    fprintf (stdout, "\n\t%s", attr->name);
  }
  fprintf (stdout, "\nList of scalar attr");  
  for (int i = edges->min; i<edges->scalars.n; ++i)
  {
    Index iscalar = edges->scalars.info[i].in_use;
    HmeshArray * attr = (HmeshArray *) edges->attr[iscalar];
    assert(attr);
    fprintf(stdout, "\n\t%s", attr->name);
  }

  hmesh_cells_destroy (edges);

  hmesh_tpool_destroy();

  hmesh_error_flush ();

  return 0;
}
