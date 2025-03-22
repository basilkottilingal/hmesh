#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

#include <time.h>

void TpoolStatus(){
  int microSec = 1;
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
  for(int k=0; k<12; ++k){
    for(int level=0; level<4; ++level) {
      for(int j=10*k; j<10*(k+1); ++j) {
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
}

int main() {

  _HmeshCells * vertices = HmeshCells(0,3);

  fflush(stdout);
  /* Insert a 'node' (vertex) to vertices */
  for(size_t i=0; i<64*HmeshTpoolBlockSize(); ++i) {
    _Node v = HmeshNodeNew(vertices);
    if( !(i%512) )
      TpoolStatus();
  }

  HmeshCellsDestroy(vertices);
 
  HmeshTpoolDestroy();

  HmeshErrorFlush(2);
 

  return 0;
}
