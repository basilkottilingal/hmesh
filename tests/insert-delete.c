#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

int main() {

  _HmeshCells * vertices = HmeshCells(0,3);

  fflush(stdout);
  /* Insert a 'node' (vertex) to vertices */
  for(_Index i=0; i<HmeshTpoolBlockSize(); ++i) {
    _Node v = HmeshNodeNew(vertices);
    fprintf(stdout, "+[%d:%d]", v.iblock, v.index);
    if(i%8 == 7) {
      if(!HmeshNodeRemove(vertices,v)) 
        fprintf(stdout, "-[%d:%d]", v.iblock, v.index);
      else
        fprintf(stdout, "-failed");
    }
  }

  HmeshCellsDestroy(vertices);
 
  HmeshTpoolDestroy();

  HmeshErrorFlush(2);
 

  return 0;
}
