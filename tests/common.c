#include <common.h>

int main() {
  hmesh_error("error checking");
  hmesh_error("Checking");
  hmesh_error_flush();
  return 0;
}
