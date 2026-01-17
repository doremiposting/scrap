#include <stdio.h>

#ifdef __APPLE__
#include "gfx11.h"
#else
#include "gfx11.h"
#endif /* */

int
main(int argc, char *argv[]) {
  if (argc > 1) { printf("%s\n", argv[1]); }
  ginit();
  render();
  gkill();
  return 0;
}
