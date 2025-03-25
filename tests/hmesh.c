#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  /* set of cells in 3D */
  _Hmesh * h = Hmesh(1, 3);

  _HmeshCells ** c[4] = { &h->p, &h->e, &h->t, &h->v };


  for(_Flag d=0; d<2; ++d) {
    _HmeshCells * cells = *c[d];
    fprintf(stdout, "\nList of attr");  
    for(int i=0; i<cells->min; ++i) {
      _HmeshArray * attr = (_HmeshArray *) cells->attr[i];
      assert(attr);
      fprintf(stdout, "\n\t%s", attr->name);
    }
    fprintf(stdout, "\nList of scalar attr");  
    for(int i = cells->min; i<cells->scalars.n; ++i) {
      _Index iscalar = cells->scalars.info[i].in_use;
      _HmeshArray * attr = (_HmeshArray *) cells->attr[iscalar];
      assert(attr);
      fprintf(stdout, "\n\t%s", attr->name);
    }
  }

  HmeshDestroy(h);

  HmeshTpoolDestroy();

  HmeshErrorFlush(2);

  return 0;
}
