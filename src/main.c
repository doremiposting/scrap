#include <stdio.h>

#ifdef __APPLE__
#include "gfx11.h"
#else
#include "gfx11.h"
#endif /* */

int
main(int argc, char *argv[]) {
  ginit();
  render();
  gkill();
  return 0;
}
