#include <common.h>

int main() {
  HmeshError("error checking");
  HmeshError("Checking");
  HmeshErrorFlush(2);
  return 0;
}
