#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  /* set of edges in 3D */
  int d = 1, D = 3;
  Hmesh * h = hmesh (d, D);

  HmeshCells ** c[4] = { &h->p, &h->e, &h->t, &h->v };

  for (int _d=0; _d<=d; ++_d)
  {
    HmeshCells * cells = *c[_d];
    fprintf (stdout, "\nList of attr");  
    for (int i=0; i<cells->min; ++i)
    {
      HmeshArray * attr = (HmeshArray *) cells->attr[i];
      assert (attr);
      fprintf (stdout, "\n\t%s", attr->name);
    }
    fprintf (stdout, "\nList of scalar attr");  
    for (int i = cells->min; i<cells->scalars.n; ++i)
    {
      Index iscalar = cells->scalars.info[i].in_use;
      HmeshArray * attr = (HmeshArray *) cells->attr[iscalar];
      assert (attr);
      fprintf (stdout, "\n\t%s", attr->name);
    }
  }

  hmesh_destroy (h);

  hmesh_tpool_destroy ();

  hmesh_error_flush ();

  return 0;
}
