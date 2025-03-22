#include <common.h>
#include <tree-pool.h>

int main(int argc, char ** argv) {
  _Index block3 = HmeshTpoolAllocate(3);  
  _Index block2 = HmeshTpoolAllocate(2);  
  _Index block1_0 = HmeshTpoolAllocate(1);  
  _Index block0_0 = HmeshTpoolAllocate(0);  
  //HmeshTpoolDeallocate(block1_0);
  _Index block1_2 = HmeshTpoolAllocate(1);  

  HmeshTpoolDestroy();

  HmeshErrorFlush(2);
  return 0;
}
