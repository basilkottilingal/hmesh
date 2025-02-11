#include <stdio.h>
#include <stdlib.h>
enum HFLAGS {
  HMESH_EDGE = 2,
  HMESH_FACE = 3,
  HMESH_HEDGE_ORDER_IN_FACE = (1|2) << HMESH_EDGE,  //
  HMESH_HEDGE_ORDER_IN_EDGE = 1 << HMESH_FACE,
  HMESH_FLAG_TEMP  = 32,
  HMESH_VERTEX_VALENCE = 1|2|4|8
};

int main(){
}
