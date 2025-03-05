#include <common.h>
#include <mempool.h>

int main() {

  /* Create a Memory pool to accomodate, blocks each of size
  .. MemblockSize() x 8 Bytes
  */
  _Mempool * pool  = Mempool(8);

  /* Get a block */
  _Memblock block1 = MempoolAllocateFrom(pool); 
  /* Anothher block */
  _Memblock block2 = MempoolAllocateFrom(pool);
  _Memblock block3 = MempoolAllocateFrom(pool);
  (void)block3;

  /* Get the memory address of the block2 */
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
 
  /* Expect No error*/ 
  HmeshError("expect no error");
  HmeshErrorFlush(2);

  /* Followig will show an error */
  MempoolDeallocateTo(block1);
  MempoolDeallocateTo(block1);
  /* There should be 'double free' error */
  HmeshErrorFlush(2);

  /* Free the pool with all the blocks */ 
  MempoolFree(pool); 

  /* Freeing pool without freeing it's block 
  .. would have produced the error */
  HmeshErrorFlush(2);

  return 0;
}
