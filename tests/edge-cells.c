#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  _HmeshCells * edges = HmeshCells(1,3);

  HmeshScalarNew(edges, "s");
  HmeshScalarNew(edges, "0"); //Error : naming should follow C naming rules
  HmeshScalarNew(edges, "v");
  HmeshScalarNew(edges, "s"); //Error : 's' already exits
  HmeshScalarNew(edges, "veryLongScalarNameIsNotAllowedFoScalarName"); //Error : long name

  HmeshErrorFlush(2);

  HmeshScalarRemove(edges, "s");
  HmeshScalarNew(edges, "s");
  HmeshScalarRemove(edges, "z"); //error : 'z' doesn't exist

  HmeshErrorFlush(2);

  fprintf(stdout, "\nList of attr");  
  for(int i=0; i<edges->min; ++i) {
    _HmeshArray * attr = (_HmeshArray *) edges->attr[i];
    assert(attr);
    fprintf(stdout, "\n\t%s", attr->name);
  }
  fprintf(stdout, "\nList of scalar attr");  
  for(int i = edges->min; i<edges->scalars.n; ++i) {
    _Index iscalar = edges->scalars.info[i].in_use;
    _HmeshArray * attr = (_HmeshArray *) edges->attr[iscalar];
    assert(attr);
    fprintf(stdout, "\n\t%s", attr->name);
  }

  HmeshCellsDestroy(edges);

  HmeshTpoolDestroy();

  HmeshErrorFlush(2);

  return 0;
}
