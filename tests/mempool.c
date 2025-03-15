#include <common.h>
#include <mempool.h>

int main() {

  HmeshError("Point-A : expect no error, till Point-B");
  /* Create a Memory pool to accomodate, blocks each of size
  .. MemblockSize() x 8 Bytes
  */
  _Mempool * pool  = Mempool(8);

  _Memblock block[3];
  /* Get a block */
  block[0] = MempoolAllocateFrom(pool); 
  /* Anothher block */
  block[1] = MempoolAllocateFrom(pool);
  block[2] = MempoolAllocateFrom(pool);
  (void) block[2];


  /* Get the memory address of the block2 */
  double * mem = (double *) MemblockAddress( block[1] );
  if(!mem)
    HmeshError("main() : Memblock() failed");
  else
    /* Iterate through each nodes  of the block */
    for(size_t i=0; i<MemblockSize(); ++i)
      mem[i] = 321.012;

  /* Free a block */ 
  MempoolDeallocateTo(block[0]);
  /* Get a block */
  block[0] = MempoolAllocateFrom(pool);
 
  /* Expect No error*/ 
  HmeshError("Point-B");
  HmeshErrorFlush(2);

  /* Followig will show an error.  */
  MempoolDeallocateTo(block[0]);
  MempoolDeallocateTo(block[0]);
  /* There should be 'double free' error */
  HmeshErrorFlush(2);

  /* Free the pool with all the blocks */ 
  MempoolFree(pool); 

  /* Freeing pool without freeing it's block 
  .. would have produced the error */
  HmeshErrorFlush(2);

  return 0;
}
