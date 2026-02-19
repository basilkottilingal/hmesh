#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main()
{
  HmeshCells * vertices = hmesh_cells (0,3);

  fflush (stdout);
  /* Insert a 'node' (vertex) to vertices */
  for (Index i=0; i<hmesh_tpool_block_size (); ++i)
  {
    Node v = hmesh_node_new (vertices);
    fprintf (stdout, "+[%d:%d]", v.iblock, v.index);
    if (i%8 == 7)
    {
      if ( !hmesh_node_remove (vertices, v)) 
        fprintf (stdout, "-[%d:%d]", v.iblock, v.index);
      else
        fprintf (stdout, "-failed");
    }
  }

  hmesh_cells_destroy (vertices);
 
  hmesh_tpool_destroy ();

  hmesh_error_flush ();

  return 0;
}
