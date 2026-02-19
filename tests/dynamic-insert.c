#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>

#include <time.h>

void TpoolStatus()
{
  int microSec = 1;
  if(microSec)
  {  
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
  uint8_t * flags = (hmesh_tpool_tree (0))->flags;
  for (int k=0; k<12; ++k)
  {
    for (int level=0; level<4; ++level)
    {
      for(int j=10*k; j<10*(k+1); ++j)
      {
        Index start = j*16;
        for (Index index = (1<<level) - 1; index < (1<<(level+1)) - 1; ++index)
        {
          Index inode = start + index; 
          char state =  flags[inode] & 128 ? 'f' :  /* free */
                      flags[inode] & 64  ? 'r' :  /* reserved */
                      '_' ;/* in use */
          fprintf (stdout, "%c", state); 
          for (int is=0; is<(1<<(3-level))-1;++is)
            fprintf (stdout,"%c", state == '_' ? '_' : ' '); 
        }
      }  
      fprintf(stdout, "\n"); 
    }
  } 
}

int main()
{

  #define PR(str) printf (str); fflush (stdout);
  PR("start");

  srand(time(0));

  HmeshCells * vertices = hmesh_cells (0,3);

  fflush (stdout);
  /* Insert a 'node' (vertex) to vertices */
  Node nodes[8];
  for (size_t i=0; i<8*hmesh_tpool_block_size (); ++i)
  {
    int inode = 0;
    for (size_t j=0; j<8; ++j)
    {
PR ("[+");
      Node node = hmesh_node_new (vertices);
      nodes[inode++] = node;
      Index x = 2, y = 3, z = 4;
      HMESH_SCALAR (vertices, x, node) = 1.0;
      HMESH_SCALAR (vertices, y, node) = 1.0;
      HMESH_SCALAR (vertices, z, node) = 1.0;
PR ("]");
    }
    hmesh_node_remove (vertices, nodes[rand()%8]);
PR ("[-]");
    if ( !(i%32) )
      TpoolStatus();
  }
PR("\ndone");

  hmesh_cells_destroy (vertices);
PR("\ncells destroyed");
 
  hmesh_tpool_destroy();
PR("\npool destroyed");

  hmesh_error_flush ();
 
  return 0;
}
