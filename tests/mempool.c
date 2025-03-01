#include <common.h>
#include <mempool.h>

int main() {
  HmeshError("error checking in mempool.");

  /* Create a Memory pool to accomodate, blocks each of size
  .. MemblockSize() x 8 Bytes
  */
  _Mempool * pool  = Mempool(8);

  /* Get a block */
  _Memblock block1 = MempoolAllocateFrom(pool); 
  /* Anothher block */
  _Memblock block2 = MempoolAllocateFrom(pool);

  /* Get the memory address of the block */
  double * mem = (double *) MemblockAddress( block2 );
  if(!mem)
    HmeshError("main() : Memblock() failed");
  else
    /* Iterate through each nodes  of the block */
    for(size_t i=0; i<MemblockSize(); ++i)
      mem[i] = 321.012;

  /* Free a block */ 
  MempoolDeallocateTo(block1);
  /* Get a block */
  block1 = MempoolAllocateFrom(pool);
 
  /* See if any function throw any error */ 
  HmeshErrorFlush(2);

  HmeshError("error checking in mempool.");
  /* Followig will show an error */
  MempoolDeallocateTo(block1);
  MempoolDeallocateTo(block1);
  /* There should be 'double free' error */
  HmeshErrorFlush(2);

  /* Free the pool with all the blocks */ 
  MempoolFree(pool); 

  return 0;
}
