#include <common.h>

int main() {
  HmeshError("error checking");
  HmeshError("Checking");
  HmeshErrorFlush(2);

  _IndexStack stack = IndexStack(10, 3, NULL);  

  for(_Index i = 0; i < 15; ++i) {
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

  /* Delete arrays */
  IndexStackDestroy(&stack);

  /* Create a new stack with attribute */
  void ** attributes = NULL;
  stack = IndexStack(10, 6, &attributes);

  for(_Index i = 0; i < 10; ++i) {
    _Index index = IndexStackFreeHead(&stack, 1);
    if(index != UINT16_MAX) 
      attributes[index] = malloc(8);
  }

  for(_Index index = 0; index < 10; ++index) {
    if(index&1) {
      free(attributes[index]);
      attributes[index] = NULL;
    }
    if(IndexStackDeallocate(&stack, index) == HMESH_ERROR)
      HmeshError("attribute of index %d still not freed", 
        index);
  }
  /* even number indices shows error */
  HmeshErrorFlush(2);

  /* Delete arrays */
  if(IndexStackDestroy(&stack) == HMESH_ERROR)
    HmeshError("attribute of index(indices) still not freed");
  /* expect error */
  HmeshErrorFlush(2);

  HmeshError("There should be no error!");
  for(_Index index = 0; index < 10; ++index) {
    if(attributes[index]) {
      free(attributes[index]);
      attributes[index] = NULL;
    }
  }
  if(IndexStackDestroy(&stack) == HMESH_ERROR)
    HmeshError("attribute of index(indices) still not freed");
  /* expect NO error */
  HmeshErrorFlush(2);

  return 0;
}
