#include <common.h>
#include <tree-pool.h>

#include <time.h>

void TpoolStatus(){
  int microSec = 50;
  if(microSec) {  
    microSec = microSec > 1000 ? 1000 : microSec;
    clock_t start_time = clock();
    clock_t wait_time = (0.001*microSec)*CLOCKS_PER_SEC ; //sleep time 
    while (clock() - start_time < wait_time) {};

    /* Clear the screen */
    printf("\033[2J");       
    /* Cursor on the left top left */
    printf("\033[1;1H");     
  }

  for(int is=0; is<8*10;++is)
    fprintf(stdout,"."); 
  fprintf(stdout, "\n"); 
  _Flag * flags = (HmeshTpoolTree(0))->flags;
  for(int level=0; level<4; ++level) {
    for(int j=0; j<10; ++j) {
      _Index start = j*16;
      for(_Index index = (1<<level) - 1; index < (1<<(level+1)) - 1; ++index) {
        _Index inode = start + index; 
        char state =  flags[inode] & 128 ? 'f' :  /* free */
                      flags[inode] & 64  ? 'r' :  /* reserved */
                      '_' ;/* in use */
        fprintf(stdout, "%c", state); 
        for(int is=0; is<(1<<(3-level))-1;++is)
          fprintf(stdout,"%c", state == '_' ? '_' : ' '); 
      }
    }  
    fprintf(stdout, "\n"); 
  } 
}

void WriteToBlock(_Index block, _Flag depth){
  /* write random info to block */
  char * mem = (char *) HmeshTpoolAddress(block);
  size_t size = 1 << (12 + HMESH_TREE_POOL_DEPTH - depth);
  while(size--)
    *mem++ = (rand() % 256) - 128;
}

#define NSTACK 15
int main(int argc, char ** argv) {

  srand(time(0));

  /* create a pool */
  HmeshTpoolAdd();

  _Index STACK[NSTACK], istack = 0, inode = 0, limit = 1000;

  while( (istack < NSTACK) && (inode < 160) && (limit--) ){
    /* print the pool */
    TpoolStatus();

    int isPush = rand() % 2;
    if(isPush && istack<NSTACK) {
      /* Add a block @ random level <= 3 */
      _Flag level = rand() % 4;
      _Index block = HmeshTpoolAllocate(level);  
      STACK[istack++] = block;
      /* rewrite the memory block with junk */
      WriteToBlock(block, level);
      
      inode = inode < (STACK[istack-1]) ? STACK[istack-1] : inode;
    }
    else if(istack){
      HmeshTpoolDeallocate(STACK[--istack]);  
    } 
    
  }

  HmeshTpoolDestroy();

  HmeshErrorFlush(2);
  return 0;
}
