#include <common.h>

int main() {
  HmeshError("error checking");
  HmeshError("Checking");
  HmeshErrorFlush(2);

  _IndexStack stack = IndexStack(10, 3, NULL);  

  for(_Index index = 0; index < 15; ++index) {
    if(IndexStackFreeHead(&stack, 1) == UINT16_MAX)
      HmeshError("Out of free index");
  }
  /* It should print the error 5 times */
  HmeshErrorFlush(2);

  for(_Index index = 5; index < 20; ++index) {
    if(IndexStackDeallocate(&stack, index) == HMESH_ERROR)
      HmeshError("index %d not in use", index);
  }
  /* indices 10-14 are not in use */
  HmeshErrorFlush(2);

  for(_Index index = 0; index < 10; ++index) {
    if(IndexStackAllocate(&stack, index) == HMESH_ERROR)
      HmeshError("index %d still in use", index);
  }
  /* indices 0-4 are still in use */
  HmeshErrorFlush(2);

  IndexStackDestroy(&stack);
  return 0;
}
