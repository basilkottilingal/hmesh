#include <common.h>
#include <mempool.h>
#include <hmesh.h>

int main() {

  _HmeshCells * vertices = HmeshCells(0,3);

  HmeshScalarNew(vertices, "s");
  HmeshScalarNew(vertices, "0"); //Error : naming should follow C naming rules
  HmeshScalarNew(vertices, "v");
  HmeshScalarNew(vertices, "s"); //Error : 's' already exits
  HmeshScalarNew(vertices, "veryLongScalarNameIsNotAllowedFoScalarName"); //Error : long name

  HmeshErrorFlush(2);

  HmeshScalarRemove(vertices, "s");
  HmeshScalarNew(vertices, "s");
  HmeshScalarRemove(vertices, "z"); //error : 'z' doesn't exist

  HmeshErrorFlush(2);

  fprintf(stdout, "\nList of attr");  
  for(int i=0; i<vertices->min; ++i) {
    _HmeshArray * attr = (_HmeshArray *) vertices->attr[i];
    assert(attr);
    fprintf(stdout, "\n\t%s", attr->name);
  }
  fprintf(stdout, "\nList of scalar attr");  
  for(int i = vertices->min; i<vertices->scalars.n; ++i) {
    _Index iscalar = vertices->scalars.info[i].in_use;
    _HmeshArray * attr = (_HmeshArray *) vertices->attr[iscalar];
    assert(attr);
    fprintf(stdout, "\n\t%s", attr->name);
  }

  HmeshCellsDestroy(vertices);

  MempoolFreeGeneral();
  HmeshErrorFlush(2);
 

  return 0;
}
