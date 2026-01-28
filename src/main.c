#include <stdio.h>

#ifdef __APPLE__
#include "gfx11.h"
#else
#include "gfxgl.h"
#include "sfxpa.h"
#endif /* */

int
main(int argc, char *argv[]) {
  if (argc > 1) { printf("%s\n", argv[1]); }
  /* TODO: sfx initialization and destruction should be handled in main.c and not gfx*.c */
  ginit();
  render();
  gkill();
  return 0;
}
