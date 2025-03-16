#include <common.h>
#include <mempool.h>

int main() {

  _Memblock b[6];
  _Flag N = 6;
  while(N--) {
    size_t obj_size = 1 << N;
    b[N] = MempoolAllocateGeneral(obj_size);
    if(!MemblockAddress(b[N]))
      HmeshError("main() : "
        "no block available for obj_size", obj_size);
    if( MempoolDeallocateGeneral(b[N]) )
      HmeshError("main() : cannot free");
  }
  if(MempoolFreeGeneral())
    HmeshError("MempoolFreeGeneral() : Error");

  HmeshError("Expect No Error");
  HmeshErrorFlush(2);

  return 0;
}
