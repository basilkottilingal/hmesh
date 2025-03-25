#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  /* set of edges in 3D */
  _Flag d = 1, D = 3;
  _Hmesh * h = Hmesh(d, D);

  _HmeshCells ** c[4] = { &h->p, &h->e, &h->t, &h->v };

  for(_Flag _d=0; _d<=d; ++_d) {
    _HmeshCells * cells = *c[_d];
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
